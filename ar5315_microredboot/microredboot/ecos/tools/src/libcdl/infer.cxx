//{{{  Banner                                   

//============================================================================
//
//      infer.cxx
//
//      Inference for common conflicts.
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1999, 2000, 2001 Red Hat, Inc.
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
// Date:        1999/11/1
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

//{{{  CdlInfer class                           

//{{{  Description                              

// ----------------------------------------------------------------------------
// The following functions provide the main entry points for inference.
//
// 1) bool CdlInfer::make_active(CdlTransaction, CdlValuable, level)
//    Do whatever it takes to make the valuable active in
//    a clean sub-transaction, if possible.
// 2) bool CdlInfer::make_inactive(CdlTransaction, CdlValuable, level)
//    Do whatever it takes to make the valuable inactive.
// 3) bool CdlInfer::set_valuable_value(CdlTransaction, CdlValuable, CdlSimpleValue&, level)
//    Try to set the valuable to the specified value, taking into
//    account the different flavors.
// 4) bool CdlInfer::set_valuable_bool(CdlTransaction, CdlValuable, bool, level)
//    Similar to (3) but deals with the boolean aspect of the valuable
//    rather than the data part.
// 5) bool CdlInfer::subexpr(CdlTransaction, CdlExpression, int index, CdlSimpleValue& goal, level)
//    Process a sub-expression and try to make it evaluate to the
//    goal.
// 6) bool CdlInfer::subexpr_bool(CdlTransaction, CdlExpression, int index, bool goal, level)
//    Ditto but only deal with boolean goals. If the expression starts to
//    involve arithmetic etc. then we need to move to CdlInfer::subexpr()
//
//    As might be expected, the sub-expression handlers contain a big
//    switch statement and calls into various auxiliary functions when
//    necessary.
//
// For convenience the various entry points check whether or not the
// desired condition is already satisfied.

//}}}
//{{{  Forward declarations                     

// ----------------------------------------------------------------------------

static bool infer_handle_interface_value(CdlTransaction, CdlInterface, CdlSimpleValue&, int);
static bool infer_handle_reference_bool(CdlTransaction, CdlValuable, bool, int);

//}}}
//{{{  CdlInfer::make_active()                  

// ----------------------------------------------------------------------------
// Making a node active. This requires the following conditions to be
// satisfied:
// 1) the parent must be made active
// 2) if the parent has flavor bool or booldata, it must be enabled
// 3) any active_if properties 

bool
CdlInfer::make_active(CdlTransaction transaction, CdlNode node, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInfer::make_active", "result %d");
    CYG_REPORT_FUNCARG3XV(transaction, node, level);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(node);

    bool result = false;
    if (transaction->is_active(node)) {
        result = true;
        CYG_REPORT_RETVAL(result);
        return result;
    }
    
    CdlContainer parent = node->get_parent();
    CYG_ASSERT_CLASSC(parent);
    if (!transaction->is_active(parent)) {
        if (!CdlInfer::make_active(transaction, parent, level)) {
            CYG_REPORT_RETVAL(result);
            return result;
        }
    }
    // The parent is now active. Does it have to be enabled as well?
    CdlValuable parent_valuable = dynamic_cast<CdlValuable>(static_cast<CdlNode>(parent));
    if (0 != parent_valuable) {
        CdlValueFlavor flavor = parent_valuable->get_flavor();
        if (((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor)) &&
            !parent_valuable->is_enabled(transaction)) {
            if (!CdlInfer::set_valuable_bool(transaction, parent_valuable, true, level)) {
                CYG_REPORT_RETVAL(result);
                return result;
            }
        }
    }
    // The parent is now active and enabled. Are there any active_if properties to worry about?
    CdlValuable valuable = dynamic_cast<CdlValuable>(node);
    if (0 != valuable) {
        std::vector<CdlProperty_GoalExpression> active_if_goals;
        std::vector<CdlProperty_GoalExpression>::iterator goal_i;
        valuable->get_active_if_conditions(active_if_goals);
        for (goal_i = active_if_goals.begin(); goal_i != active_if_goals.end(); goal_i++) {
            
            CdlEvalContext context(transaction, valuable, *goal_i);
            try {
                if (!(*goal_i)->eval(context)) {
                    CdlExpression expr = (*goal_i)->get_expression();
                    if (!CdlInfer::subexpr_bool(transaction, expr, expr->first_subexpression, true, level)) {
                        CYG_REPORT_RETVAL(result);
                        return result;
                    }
                }
            } catch(...) {
                CYG_REPORT_RETVAL(result);
                return result;
            }
        }
        
    }
    
    result = transaction->is_active(node);
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlInfer::make_inactive()                

// ----------------------------------------------------------------------------
// Making a node inactive can be done in three ways:
// 1) if the parent is boolean, try disabling it
// 2) otherwise if the parent can be made inactive, that will do
//    fine.
// 3) if there are any active_if properties, they could be considered
//    as well. For now this possibility is ignored.

bool
CdlInfer::make_inactive(CdlTransaction transaction, CdlNode node, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInfer::make_inactive", "result %d");
    CYG_REPORT_FUNCARG3XV(transaction, node, level);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(node);

    bool result = false;
    if (!transaction->is_active(node)) {
        result = true;
        CYG_REPORT_RETVAL(result);
        return result;
    }

    CdlContainer parent = node->get_parent();
    if (0 == parent) {
        // No point in trying to disable the entire configuration.
        CYG_REPORT_RETVAL(result);
        return result;
    }
    
    CdlValuable  parent_valuable = dynamic_cast<CdlValuable>(parent);
    if (0 != parent_valuable) {
        CdlValueFlavor flavor = parent_valuable->get_flavor();
        if ((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor)) {
            // Since the current node is active the parent must currently be enabled.
            // A sub-transaction is needed because an alternative approach is
            // possible later on.
            CdlTransaction subtransaction = transaction->make(transaction->get_conflict());
            if (CdlInfer::set_valuable_bool(subtransaction, parent_valuable, false, level)) {
                subtransaction->commit();
                delete subtransaction;
                result = true;
                CYG_REPORT_RETVAL(result);
                return result;
            } else {
                subtransaction->cancel();
                delete subtransaction;
            }
        }
    }

    // It is not possible to disable the parent. How about making it inactive?
    if (CdlInfer::make_inactive(transaction, parent, level)) {
        result = true;
        CYG_REPORT_RETVAL(result);
        return result;
    }

    // For now do not try to mess about with active_if conditions.
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlInfer::set_valuable_value()           

// ----------------------------------------------------------------------------
// Deal with the value part of a valuable. The valuable is known to exist
// and be active, so this code only deals with the actual value part.

bool
CdlInfer::set_valuable_value(CdlTransaction transaction, CdlValuable valuable, CdlSimpleValue& goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInfer::set_valuable_value", "result %d");
    CYG_REPORT_FUNCARG3XV(transaction, valuable, level);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(valuable);
    CYG_PRECONDITIONC(transaction->is_active(valuable));

    bool result = false;

    const CdlValue& current_value = transaction->get_whole_value(valuable);
    CdlValueFlavor  flavor        = current_value.get_flavor();
    bool bool_goal                = goal.get_bool_value();

    switch(flavor) {
      default                  :
      case CdlValueFlavor_None :
          break;

      case CdlValueFlavor_Bool :
          if (bool_goal == current_value.is_enabled()) {
              result = true;
          } else {
              if (valuable->is_modifiable() &&
                  (0 == dynamic_cast<CdlLoadable>(valuable)) &&
                  !transaction->changed_by_user(valuable)) {

                  valuable->set_enabled(transaction, bool_goal, CdlValueSource_Inferred);
                  valuable->set_source(transaction, CdlValueSource_Inferred);
                  result = transaction->resolve_recursion(level);
              }

          }
          break;

      case CdlValueFlavor_BoolData :
          if (!bool_goal && !current_value.is_enabled()) {
              result = true;
          } else if (bool_goal && current_value.is_enabled() && (goal == current_value.get_simple_value())) {
              result = true;
          } else {
              if (valuable->is_modifiable() &&
                  (0 == dynamic_cast<CdlLoadable>(valuable)) &&
                  !transaction->changed_by_user(valuable)) {
                  
                  if (!bool_goal) {
                      valuable->disable(transaction, CdlValueSource_Inferred);
                  } else {
                      valuable->enable_and_set_value(transaction, goal, CdlValueSource_Inferred);
                  }
                  valuable->set_source(transaction, CdlValueSource_Inferred);
                  result = transaction->resolve_recursion(level);
              } else if (0 != dynamic_cast<CdlInterface>(valuable)) {
                  // Interfaces are not directly modifiable, but their implementors are.
                  result = infer_handle_interface_value(transaction, dynamic_cast<CdlInterface>(valuable), goal, level);
              }
          }
          break;

      case CdlValueFlavor_Data:
        // Now check whether or not the valuable already has the desired value
        if (goal == current_value.get_simple_value()) {
            result = true;
        } else {
            if (valuable->is_modifiable() &&
                (0 == dynamic_cast<CdlLoadable>(valuable)) &&
                !transaction->changed_by_user(valuable)) {
    
                // Make the change, propagate, and perform further resolution.
                valuable->set_value(transaction, goal, CdlValueSource_Inferred);
                valuable->set_source(transaction, CdlValueSource_Inferred);
                result = transaction->resolve_recursion(level);
            } else if (0 != dynamic_cast<CdlInterface>(valuable)) {
                // Interfaces are not directly modifiable, but their implementors are.
                result = infer_handle_interface_value(transaction, dynamic_cast<CdlInterface>(valuable), goal, level);
            }
        }
        break;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlInfer::set_valuable_bool()            

// ----------------------------------------------------------------------------
// Deal with the boolean part of a valuable. It is assumed that active vs.
// inactive is dealt with elsewhere so this code only needs to worry
// about the valuable itself.

bool
CdlInfer::set_valuable_bool(CdlTransaction transaction, CdlValuable valuable, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInfer::set_valuable_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, valuable, goal, level);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(valuable);

    bool result = false;

    // Examine the current flavor. If None or Data then the valuable
    // is always enabled. If BoolData or Boolean then the condition
    // may be satisfied already, otherwise an attempt must be made
    // to change the value and see what happens.
    CdlValueFlavor flavor = valuable->get_flavor();
    if (CdlValueFlavor_None == flavor) {
        if (goal) {
            result = true;
        }
        CYG_REPORT_RETVAL(result);
        return result;
    }

    if (CdlValueFlavor_Data == flavor) {
        std::string value = valuable->get_value(transaction);
        if (goal) {
            if (("" != value) && ("0" != value)) {
                result = true;
            }
        } else {
            if (("" == value) || ("0" == value)) {
                result = true;
            }
        }
        CYG_REPORT_RETVAL(result);
        return result;
    }
    
    CYG_ASSERTC((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor));
    bool enabled = valuable->is_enabled(transaction);
    if (enabled == goal) {
        result = true;
        CYG_REPORT_RETVAL(result);
        return result;
    }

    // enabled != goal, and we have a boolean or booldata item.
    // Before we actually try making any changes, is this sensible?
    if (!valuable->is_modifiable() ||
        (0 != dynamic_cast<CdlLoadable>(valuable)) ||
        transaction->changed_by_user(valuable)) {

        CYG_REPORT_RETVAL(result);
        return result;
    }
    // If we are about to disable a container, better check that this would
    // not annoy the user either
    if (!goal) {
        CdlContainer container = dynamic_cast<CdlContainer>(valuable);
        if ((0 != container) && transaction->subnode_changed_by_user(container)) {
            CYG_REPORT_RETVAL(result);
            return result;
        }
    }
    
    // Try to change the state, propagate, and perform further resolution.
    valuable->set_enabled(transaction, goal, CdlValueSource_Inferred);
    valuable->set_source(transaction, CdlValueSource_Inferred);
    result = transaction->resolve_recursion(level);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  infer_choose()                           

// ----------------------------------------------------------------------------
// Given two sub-transactions which may or may not have succeeded, pick the
// preferred one. This happens for many binary operators.

static bool
infer_lhs_preferable(CdlTransaction lhs_transaction, bool lhs_result, CdlTransaction rhs_transaction, bool rhs_result)
{
    CYG_REPORT_FUNCNAMETYPE("infer_choose2", "result %d");
    CYG_REPORT_FUNCARG4XV(lhs_transaction, lhs_result, rhs_transaction, rhs_result);
    CYG_PRECONDITIONC(lhs_result || rhs_result);
    
    bool result = true;
    
    if (lhs_result && !rhs_result) {
        // Only the lhs succeeded.
        result = true;
    } else if (!lhs_result && rhs_result) {
        // Only the rhs succeeded.
        result = false;
    } else if (lhs_result && rhs_result) {
        // Both sides succeeded. Next check for user_confirmation.
        bool lhs_confirm_needed = lhs_transaction->user_confirmation_required();
        bool rhs_confirm_needed = rhs_transaction->user_confirmation_required();
        if (lhs_confirm_needed && !rhs_confirm_needed) {
            result = false;
        } else if (!lhs_confirm_needed && rhs_confirm_needed) {
            result = true;
        } else {
            // Neither or both of the two sides need user confirmation, so they
            // are equal in that respect
            if (lhs_transaction->is_preferable_to(rhs_transaction)) {
                result = true;
            } else {
                result = false;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// A variant which will actually do the commits and cancels. This is
// commonly required when doing inferences of binary operators.
static bool
infer_choose2(CdlTransaction lhs_transaction, bool lhs_result, CdlTransaction rhs_transaction, bool rhs_result)
{
    CYG_REPORT_FUNCNAMETYPE("infer_choose2", "result %d");
    CYG_REPORT_FUNCARG4XV(lhs_transaction, lhs_result, rhs_transaction, rhs_result);
    bool result = false;

    if (lhs_result || rhs_result) {
        bool lhs_preferable = infer_lhs_preferable(lhs_transaction, lhs_result, rhs_transaction, rhs_result);
        if (lhs_preferable) {
            rhs_transaction->cancel();
            lhs_transaction->commit();
        } else {
            lhs_transaction->cancel();
            rhs_transaction->commit();
        }
        result = true;
    } else {
        // Neither side succeeded.
        lhs_transaction->cancel();
        rhs_transaction->cancel();
    }
    
    // Zero or one of these transactions will have been committed,
    // neither is still necessary.
    delete lhs_transaction;
    delete rhs_transaction;

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  infer_handle_interface()                 

// ----------------------------------------------------------------------------
// Set an interface to a specific value, which should be some number n.
// If (n == 0) then all implementers must be disabled or made inactive.
// If (n == 1) then exactly one of the implementers must be active and enabled.
// Other combinations are not considered here, they could lead to an
// exponential explosion.

static bool
infer_handle_interface_value(CdlTransaction transaction, CdlInterface interface, CdlSimpleValue& goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_reference_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, interface, &goal, level);
    bool result = false;

    if (goal.has_integer_value()) {
        cdl_int real_goal = goal.get_integer_value();
        if (real_goal == interface->get_integer_value(transaction)) {
            result = true;
        } else if (0 == real_goal) {
            // All implementers must be disabled or made inactive. This
            // can be achieved by creating a sub-transaction and calling
            // infer_handle_reference_bool() on all of the implementers.
            //
            // However there are no guarantees that the result is what
            // is intended. Updating a later implementer may as a side
            // effect cause an earlier one to become active again. Also
            // there may be confusion with valuables with the data
            // flavor being given a value of 0. Hence a final check is
            // needed that the new interface value really is the desired goal.
            CdlTransaction sub_transaction;
            std::vector<CdlValuable> implementers;
            std::vector<CdlValuable>::const_iterator impl_i;

            sub_transaction = transaction->make(transaction->get_conflict());
            try {
                interface->get_implementers(implementers);
                for (impl_i = implementers.begin(); impl_i != implementers.end(); impl_i++) {
                    (void) infer_handle_reference_bool(sub_transaction, *impl_i, false, level);
                }
                if (0 == interface->get_integer_value(sub_transaction)) {
                    sub_transaction->commit();
                    result = true;
                } else {
                    sub_transaction->cancel();
                }
            } catch (...) {
                delete sub_transaction;
                throw;
            }
            delete sub_transaction;
            sub_transaction = 0;
            
        } else if (1 == real_goal) {
            // This is a bit trickier than the above. We need n
            // sub-transactions, one per implementer. In each
            // sub-transaction we try to set exactly one of the
            // implementers to enabled and the rest to disabled.
            std::vector<CdlValuable>    implementers;
            unsigned int                impl_count;
            unsigned int                i, j;
            
            interface->get_implementers(implementers);
            impl_count = implementers.size();
            std::vector<CdlTransaction> sub_transactions;
            std::vector<bool>           results;

            try {
                for (i = 0; i < impl_count; i++) {
                    CdlTransaction sub_transaction = transaction->make(transaction->get_conflict());
                    sub_transactions.push_back(sub_transaction);
                    results.push_back(false);
                    results[i]          = false;
                }
                for (i = 0; i < impl_count; i++) {
                    for (j = 0; j < impl_count; j++) {
                        (void) infer_handle_reference_bool(sub_transactions[i], implementers[j], (i == j), level);
                    }
                    if (1 == interface->get_integer_value(sub_transactions[i])) {
                        results[i] = true;
                    }
                }
                
                // At this point we may have some combination of successful and unsucessful
                // sub-transactions, and it is time to choose the best one.
                CdlTransaction  preferred = 0;
                for (i = 0; i < impl_count; i++) {
                    if (results[i]) {
                        preferred = sub_transactions[i];
                        break;
                    }
                }

                for (j = i + 1; j < impl_count; j++) {
                    if (results[j]) {
                        if (!infer_lhs_preferable(preferred, true, sub_transactions[j], true)) {
                            preferred = sub_transactions[j];
                        }
                    }
                }

                // Now either preferred == 0, i.e. all
                // sub-transactions failed and we want to cancel them
                // all. Or we have a viable sub-transaction.
                for (i = 0; i < impl_count; i++) {
                    if (preferred == sub_transactions[i]) {
                        sub_transactions[i]->commit();
                        result = true;
                    } else {
                        sub_transactions[i]->cancel();
                    }
                    delete sub_transactions[i];
                    sub_transactions[i] = 0;
                }
                
            } catch(...) {
                for (i = 0; i < sub_transactions.size(); i++) {
                    if (0 != sub_transactions[i]) {
                        sub_transactions[i]->cancel();
                        delete sub_transactions[i];
                        sub_transactions[i] = 0;
                    }
                }
            }
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  infer_handle_reference()                 

// ----------------------------------------------------------------------------
// We are processing an expression and have reached a point where we
// need <reference>, !<reference> or <reference>==<value>. The
// reference may currently be unbound, in which case 0 is the only
// goal that can be satisfied. If the reference is bound then it may
// be possible to satisfy the goal by setting the value. In addition
// it is necessary to worry about active vs. inactive state.

static bool
infer_handle_reference_bool(CdlTransaction transaction, CdlValuable valuable, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_reference_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, valuable, goal, level);

    bool result = false;
    if (0 == valuable) {
        if (!goal) {
            result = true;
        }
        CYG_REPORT_RETVAL(result);
        return result;
    }

    // If the valuable should evaluate to true then it must be both active
    // and be either enabled or have a non-zero value.
    if (goal) {
        if (!transaction->is_active(valuable)) {
            if (!CdlInfer::make_active(transaction, valuable, level)) {
                CYG_REPORT_RETVAL(result);
                return result;
            }
        }
        if (CdlInfer::set_valuable_bool(transaction, valuable, true, level)) {
            result = true;
        }

    } else {
        // If the valuable should evaluate to false then it must be either
        // inactive or it must be disabled or have a zero value.
        if (!transaction->is_active(valuable)) {
            // The goal is already satisfied, no need to proceed
            result = true;
            CYG_REPORT_RETVAL(result);
            return result;
        }
        
        // There is a choice to be made so two sub-transactions are
        // needed. Disabling is generally preferred to making inactive.
        CdlTransaction value_transaction    = transaction->make(transaction->get_conflict());
        CdlTransaction inactive_transaction = 0;
        bool value_result = CdlInfer::set_valuable_bool(value_transaction, valuable, false, level);
        if (value_result && !value_transaction->user_confirmation_required()) {
            value_transaction->commit();
            delete value_transaction;
            value_transaction = 0;
            result = true;
            CYG_REPORT_RETVAL(result);
            return result;
        }

        inactive_transaction = transaction->make(transaction->get_conflict());
        bool inactive_result = CdlInfer::make_inactive(inactive_transaction, valuable, level);
        if (!inactive_result) {
            if (value_result) {
                // Changing the value is the only solution.
                inactive_transaction->cancel();
                value_transaction->commit();
                result = true;
            } else {
                inactive_transaction->cancel();
                value_transaction->cancel();
                result = false;
            }
        } else {
            if (!value_result) {
                // Making the valuable inactive is the only solution.
                value_transaction->cancel();
                inactive_transaction->commit();
                result = true;
            } else if (!inactive_transaction->user_confirmation_required()) {
                // Disabling the valuable would require user confirmation, making it inactive does not
                value_transaction->cancel();
                inactive_transaction->commit();
                result = true;
            } else {
                // Both approaches are valid but would require user confirmation.
                // Pick the preferred one.
                if (value_transaction->is_preferable_to(inactive_transaction)) {
                    inactive_transaction->cancel();
                    value_transaction->commit();
                    result = true;
                } else {
                    value_transaction->cancel();
                    inactive_transaction->commit();
                    result = true;
                }
            }
        }

        delete value_transaction;
        delete inactive_transaction;
        value_transaction = 0;
        inactive_transaction = 0;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Try to set a valuable to a particular value. Of course the reference
// may not be bound yet.
//
// First check whether or not the valuable is currently active. If it is
// inactive and the goal is 0 then we have succeeded. If it is active and
// the goal is 0 then we could try to make the valuable inactive, but
// this possibility is ignored for now in case it leads to unexpected
// behaviour. If it is active then we try to set the value, using
// CdlInfer::set_valuable_value().

static bool
infer_handle_reference_value(CdlTransaction transaction, CdlValuable valuable, CdlSimpleValue& goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_reference", "result %d");
    CYG_REPORT_FUNCARG3XV(transaction, valuable, level);

    bool result = false;

    if (0 == valuable) {
        if (goal == (cdl_int) 0) {
            result = true;
        }
    } else {
    
        bool active = transaction->is_active(valuable);
        if (!active) {
            if (goal == (cdl_int) 0) {
                result = true;
            }
        } else {
            result = CdlInfer::set_valuable_value(transaction, valuable, goal, level);
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  infer_handle_xxx_constant()              

// ----------------------------------------------------------------------------
// Somewhere in the expression processing we have encountered a string
// constant. The expression cannot be changed, so either the goal matches
// the constant or it does not.
static bool
infer_handle_string_constant_bool(CdlSimpleValue& constant, bool goal)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_string_constant_bool", "result %d");
    bool result = false;

    if (goal) {
        if (("" != constant.get_value()) && ("0" != constant.get_value())) {
            result = true;
        }
    } else {
        if (("" == constant.get_value()) || ("0" == constant.get_value())) {
            result = true;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
infer_handle_string_constant_value(CdlSimpleValue& constant, CdlSimpleValue& goal)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_string_constant_value", "result %d");
    bool result = false;

    if (constant.get_value() == goal.get_value()) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Integers are also fairly straightforward.
static bool
infer_handle_integer_constant_bool(CdlSimpleValue& constant, bool goal)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_integer_constant_bool", "result %d");
    CYG_PRECONDITIONC(constant.has_integer_value());
    bool result = false;

    if (goal) {
        if (0 != constant.get_integer_value()) {
            result = true;
        }
    } else {
        if (0 == constant.get_integer_value()) {
            result = true;
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
infer_handle_integer_constant_value(CdlSimpleValue& constant, CdlSimpleValue& goal)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_integer_constant_value", "result %d");
    CYG_PRECONDITIONC(constant.has_integer_value());
    bool result = false;

    if (goal.has_integer_value() && (constant.get_integer_value() == goal.get_integer_value())) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Doubles are also straightforward, except than an exact comparision may
// be too strict. There is not a lot that can be done about this right now.
// Future enhancements to CDL may support tolerances.
static bool
infer_handle_double_constant_bool(CdlSimpleValue& constant, bool goal)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_double_constant_bool", "result %d");
    CYG_PRECONDITIONC(constant.has_double_value());
    bool result = false;

    if (goal) {
        if (0.0 != constant.get_double_value()) {
            result = true;
        }
    } else {
        if (0.0 == constant.get_double_value()) {
            result = true;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
infer_handle_double_constant_value(CdlSimpleValue& constant, CdlSimpleValue& goal)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_double_constant_value", "result %d");
    CYG_PRECONDITIONC(constant.has_double_value());
    bool result = false;

    if (goal.has_double_value() && (constant.get_double_value() == goal.get_double_value())) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  infer_handle_logical_xxx()               

// ----------------------------------------------------------------------------
// Logical not simply involves inverting the goal and then trying to infer
// the rest of the sub-expression. There is little point in touching
// the other arguments.
static bool
infer_handle_logical_NOT_bool(CdlTransaction transaction, CdlExpression expr, unsigned int index, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_logical_NOT_bool", "result %d");

    bool result = CdlInfer::subexpr_bool(transaction, expr, index, !goal, level);
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Depending on the goal, we want either both sides of the AND to evaluate to
// true, or we want one of the sides to evaluate to false.
static bool
infer_handle_AND_bool(CdlTransaction transaction, CdlExpression expr, unsigned int lhs, unsigned int rhs,
                      bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_AND_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, expr, lhs, rhs);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC(lhs != rhs);

    bool result = false;
    
    if (goal) {
        // Both sides must be true in the same transaction, in case
        // the solutions overlap in conflicting ways. A sub-transaction
        // is still used to avoid polluting current values if the lhs
        // can be inferred but not the rhs.
        CdlTransaction sub_transaction = transaction->make(transaction->get_conflict());
        if (CdlInfer::subexpr_bool(sub_transaction, expr, lhs, true, level) &&
            CdlInfer::subexpr_bool(sub_transaction, expr, rhs, true, level)) {
            sub_transaction->commit();
            result = true;
        } else {
            sub_transaction->cancel();
        }
        delete sub_transaction;
    } else {
        // We need to try out both sides of the OR and see which one is preferable.
        // An optimization would be to only try the LHS, but trying both allows
        // for a more informed choice.
        CdlTransaction lhs_transaction = transaction->make(transaction->get_conflict());
        CdlTransaction rhs_transaction = transaction->make(transaction->get_conflict());
        bool lhs_result = CdlInfer::subexpr_bool(lhs_transaction, expr, lhs, false, level);
        bool rhs_result = CdlInfer::subexpr_bool(rhs_transaction, expr, rhs, false, level);

        result = infer_choose2(lhs_transaction, lhs_result, rhs_transaction, rhs_result);
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// The support for the other logical operations involves basically minor
// variants of the above.

static bool
infer_handle_OR_bool(CdlTransaction transaction, CdlExpression expr, unsigned int lhs, unsigned int rhs,
                     bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_OR_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, expr, lhs, rhs);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC(lhs != rhs);


    bool result = false;
    
    if (goal) {
        // We need to try out both sides of the OR and see which one is preferable.
        // An optimization would be to only try the LHS, but trying both allows
        // for a more informed choice.
        CdlTransaction lhs_transaction = transaction->make(transaction->get_conflict());
        CdlTransaction rhs_transaction = transaction->make(transaction->get_conflict());
        bool lhs_result = CdlInfer::subexpr_bool(lhs_transaction, expr, lhs, true, level);
        bool rhs_result = CdlInfer::subexpr_bool(rhs_transaction, expr, rhs, true, level);

        result = infer_choose2(lhs_transaction, lhs_result, rhs_transaction, rhs_result);
    } else {
        
        // !(A || B) -> !A && !B
        CdlTransaction sub_transaction = transaction->make(transaction->get_conflict());
        if (CdlInfer::subexpr_bool(sub_transaction, expr, lhs, false, level) &&
            CdlInfer::subexpr_bool(sub_transaction, expr, rhs, false, level)) {
            sub_transaction->commit();
            result = true;
        } else {
            sub_transaction->cancel();
        }
        delete sub_transaction;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------

static bool
infer_handle_IMPLIES_bool(CdlTransaction transaction, CdlExpression expr, unsigned int lhs, unsigned int rhs,
                     bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_implies_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, expr, lhs, rhs);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC(lhs != rhs);


    bool result = false;
    
    if (goal) {
        // A implies B -> !A || B
        // Given a choice between !A or B, arguably the "implies"
        // operator has the connotation that B is preferred. All other
        // things being equal, infer_choose2() will prefer the rhs
        // over the lhs so this is achieved automagically.
        
        CdlTransaction lhs_transaction = transaction->make(transaction->get_conflict());
        CdlTransaction rhs_transaction = transaction->make(transaction->get_conflict());
        bool lhs_result = CdlInfer::subexpr_bool(lhs_transaction, expr, lhs, false, level);
        bool rhs_result = CdlInfer::subexpr_bool(rhs_transaction, expr, rhs, true, level);

        result = infer_choose2(lhs_transaction, lhs_result, rhs_transaction, rhs_result);
        
    } else {
        
        // !(A implies B) -> !(!A || B) -> (A && !B)
        CdlTransaction sub_transaction = transaction->make(transaction->get_conflict());
        if (CdlInfer::subexpr_bool(sub_transaction, expr, lhs, true, level) &&
            CdlInfer::subexpr_bool(sub_transaction, expr, rhs, false, level)) {
            sub_transaction->commit();
            result = true;
        } else {
            sub_transaction->cancel();
        }
        delete sub_transaction;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------

static bool
infer_handle_XOR_bool(CdlTransaction transaction, CdlExpression expr, unsigned int lhs, unsigned int rhs,
                     bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_XOR_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, expr, lhs, rhs);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC(lhs != rhs);


    bool result = false;
    
    if (goal) {
        // (A xor B) -> (A && !B) || (!A && B)

        CdlTransaction sub1 = transaction->make(transaction->get_conflict());
        CdlTransaction sub2 = transaction->make(transaction->get_conflict());
        bool result1 = (CdlInfer::subexpr_bool(sub1, expr, lhs, true, level) &&
                        CdlInfer::subexpr_bool(sub1, expr, rhs, false, level));
        bool result2 = (CdlInfer::subexpr_bool(sub2, expr, lhs, false, level) &&
                        CdlInfer::subexpr_bool(sub2, expr, rhs, true, level));
                           
        result = infer_choose2(sub1, result1, sub2, result2);
        
    } else {
        
        // !(A xor B) -> (!A && !B) || (A && B)
        CdlTransaction sub1 = transaction->make(transaction->get_conflict());
        CdlTransaction sub2 = transaction->make(transaction->get_conflict());
        bool result1 = (CdlInfer::subexpr_bool(sub1, expr, lhs, false, level) &&
                        CdlInfer::subexpr_bool(sub1, expr, rhs, false, level));
        bool result2 = (CdlInfer::subexpr_bool(sub2, expr, lhs, true, level) &&
                        CdlInfer::subexpr_bool(sub2, expr, rhs, true, level));
                           
        result = infer_choose2(sub1, result1, sub2, result2);
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------

static bool
infer_handle_EQV_bool(CdlTransaction transaction, CdlExpression expr, unsigned int lhs, unsigned int rhs,
                     bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_EQV_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, expr, lhs, rhs);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC(lhs != rhs);


    bool result = false;
    
    if (goal) {
        // (A eqv B) -> (A && B) || (!A && !B)

        CdlTransaction sub1 = transaction->make(transaction->get_conflict());
        CdlTransaction sub2 = transaction->make(transaction->get_conflict());
        bool result1 = (CdlInfer::subexpr_bool(sub1, expr, lhs, true, level) &&
                        CdlInfer::subexpr_bool(sub1, expr, rhs, true, level));
        bool result2 = (CdlInfer::subexpr_bool(sub2, expr, lhs, false, level) &&
                        CdlInfer::subexpr_bool(sub2, expr, rhs, false, level));
                           
        result = infer_choose2(sub1, result1, sub2, result2);
    } else {
        // !(A eqv B) -> (A && !B) || (!A && B)

        CdlTransaction sub1 = transaction->make(transaction->get_conflict());
        CdlTransaction sub2 = transaction->make(transaction->get_conflict());
        bool result1 = (CdlInfer::subexpr_bool(sub1, expr, lhs, true, level) &&
                        CdlInfer::subexpr_bool(sub1, expr, rhs, false, level));
        bool result2 = (CdlInfer::subexpr_bool(sub2, expr, lhs, false, level) &&
                        CdlInfer::subexpr_bool(sub2, expr, rhs, true, level));
                           
        result = infer_choose2(sub1, result1, sub2, result2);
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  infer_handle_Equal()                     

// ----------------------------------------------------------------------------
// Handle expressions of the form A == B. This can be achieved either by
// evaluating B and trying to assign the result to A, or vice versa. There
// is a problem if assigning to one side has a side effect on the other, e.g.
//
//   requires { xyzzy == (xyzzy + 3) }
//
// This has to be guarded against by reevaluating the expression.
//
// At present this code only copes with equality, not inequality.

static bool
infer_handle_equal_bool(CdlTransaction transaction, CdlExpression expr, unsigned int lhs, unsigned int rhs, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_equal_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, expr, lhs, rhs);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC(lhs != rhs);

    bool result = false;
    if (goal) {
        
        // We need two sub-transactions, The lhs_transaction is for evaluating the lhs
        // and trying to update the rhs. 
        CdlTransaction  lhs_transaction = transaction->make(transaction->get_conflict());
        bool            lhs_result = false;
        try {
            CdlSimpleValue  lhs_value;
            CdlEvalContext  lhs_context(lhs_transaction);
            expr->eval_subexpression(lhs_context, lhs, lhs_value);
            lhs_result = CdlInfer::subexpr_value(lhs_transaction, expr, rhs, lhs_value, level);
            if (lhs_result) {
                CdlSimpleValue check;
                expr->eval_subexpression(lhs_context, lhs, check);
                if (lhs_value != check) {
                    lhs_result = false;
                }
            }
        } catch (...) {
            lhs_result = false;
        }
        
        CdlTransaction  rhs_transaction = transaction->make(transaction->get_conflict());
        bool            rhs_result = false;
        try {
            CdlSimpleValue  rhs_value;
            CdlEvalContext  rhs_context(rhs_transaction);
            expr->eval_subexpression(rhs_context, rhs, rhs_value);
            rhs_result = CdlInfer::subexpr_value(rhs_transaction, expr, lhs, rhs_value, level);
            if (rhs_result) {
                CdlSimpleValue check;
                expr->eval_subexpression(rhs_context, rhs, check);
                if (rhs_value != check) {
                    rhs_result = false;
                }
            }
        } catch (...) {
            rhs_result = false;
        }

        result = infer_choose2(lhs_transaction, lhs_result, rhs_transaction, rhs_result);
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  infer_handle_numerical_equal()           

// ----------------------------------------------------------------------------
// Handle expressions of the form A == B, where the comparison has to be
// numerical in basis. This is used primarily for operators like <=
// and >.

static bool
infer_handle_numerical_equal_bool(CdlTransaction transaction, CdlExpression expr, unsigned int lhs, unsigned int rhs, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("infer_handle_numerical_equal_bool", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, expr, lhs, rhs);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC(lhs != rhs);

    bool result = false;
    if (goal) {
        
        // We need two sub-transactions, The lhs_transaction is for evaluating the lhs
        // and trying to update the rhs. 
        CdlTransaction  lhs_transaction = transaction->make(transaction->get_conflict());
        bool            lhs_result = false;
        try {
            CdlSimpleValue  lhs_value;
            CdlEvalContext  lhs_context(lhs_transaction);
            expr->eval_subexpression(lhs_context, lhs, lhs_value);
            if (lhs_value.has_integer_value() || lhs_value.has_double_value()) {
                lhs_result = CdlInfer::subexpr_value(lhs_transaction, expr, rhs, lhs_value, level);
                if (lhs_result) {
                    CdlSimpleValue check;
                    expr->eval_subexpression(lhs_context, lhs, check);
                    if (lhs_value != check) {
                        lhs_result = false;
                    }
                }
            }
        } catch (...) {
            lhs_result = false;
        }
        
        CdlTransaction  rhs_transaction = transaction->make(transaction->get_conflict());
        bool            rhs_result = false;
        try {
            CdlSimpleValue  rhs_value;
            CdlEvalContext  rhs_context(rhs_transaction);
            expr->eval_subexpression(rhs_context, rhs, rhs_value);
            if (rhs_value.has_integer_value() || rhs_value.has_double_value()) {
                rhs_result = CdlInfer::subexpr_value(rhs_transaction, expr, lhs, rhs_value, level);
                if (rhs_result) {
                    CdlSimpleValue check;
                    expr->eval_subexpression(rhs_context, rhs, check);
                    if (rhs_value != check) {
                        rhs_result = false;
                    }
                }
            }
        } catch (...) {
            rhs_result = false;
        }

        result = infer_choose2(lhs_transaction, lhs_result, rhs_transaction, rhs_result);
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlInfer::subexpr_bool()                 

// ----------------------------------------------------------------------------
bool
CdlInfer::subexpr_bool(CdlTransaction transaction, CdlExpression expr, unsigned int index, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInfer::subexpr_bool", "result %d");
    CYG_REPORT_FUNCARG5XV(transaction, expr, index, goal, level);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC((0 <= index) && (index < expr->sub_expressions.size()));
    
    bool result = false;
    CdlSubexpression& subexpr = expr->sub_expressions[index];

    switch(subexpr.op) {
        
      case CdlExprOp_Reference :
          // The most common case. Follow the reference, and call the appropriate function.
          // Note that the reference may be unbound.
          {
              CdlNode node = expr->references[subexpr.reference_index].get_destination();
              CdlValuable valuable = 0;
              if (0 != node) {
                  valuable = dynamic_cast<CdlValuable>(node);
              }
              result = infer_handle_reference_bool(transaction, valuable, goal, level);
              break;
          }
          
      case CdlExprOp_StringConstant :
          result = infer_handle_string_constant_bool(subexpr.constants, goal);
          break;
                                                
      case CdlExprOp_IntegerConstant :
          result = infer_handle_integer_constant_bool(subexpr.constants, goal);
          break;
          
      case CdlExprOp_DoubleConstant :
          result = infer_handle_double_constant_bool(subexpr.constants, goal);
          break;

      case CdlExprOp_LogicalNot :
          result = infer_handle_logical_NOT_bool(transaction, expr, subexpr.lhs_index, goal, level);
          break;
          
      case CdlExprOp_And :
          result = infer_handle_AND_bool(transaction, expr, subexpr.lhs_index, subexpr.rhs_index, goal, level);
          break;
          
      case CdlExprOp_Or :
          result = infer_handle_OR_bool(transaction, expr, subexpr.lhs_index, subexpr.rhs_index, goal, level);
          break;

      case CdlExprOp_Implies :
          result = infer_handle_IMPLIES_bool(transaction, expr, subexpr.lhs_index, subexpr.rhs_index, goal, level);
          break;
        
      case CdlExprOp_Xor :
          result = infer_handle_XOR_bool(transaction, expr, subexpr.lhs_index, subexpr.rhs_index, goal, level);
          break;
        
      case CdlExprOp_Eqv :
          result = infer_handle_EQV_bool(transaction, expr, subexpr.lhs_index, subexpr.rhs_index, goal, level);
          break;
        
      case CdlExprOp_Equal :
          result = infer_handle_equal_bool(transaction, expr, subexpr.lhs_index, subexpr.rhs_index, goal, level);
          break;

      case CdlExprOp_NotEqual :
          result = infer_handle_equal_bool(transaction, expr, subexpr.lhs_index, subexpr.rhs_index, !goal, level);
          break;

          // <= is satisfied by a numerical equality. However the inverse relation > cannot be handled that way
          // The other comparison operators are much the same.
      case CdlExprOp_LessEqual :
      case CdlExprOp_GreaterEqual :
          if (goal) {
              result = infer_handle_numerical_equal_bool(transaction, expr, subexpr.lhs_index, subexpr.rhs_index, true, level);
          }
          break;

      case CdlExprOp_LessThan :
      case CdlExprOp_GreaterThan :
          if (!goal) {
              result = infer_handle_numerical_equal_bool(transaction, expr, subexpr.lhs_index, subexpr.rhs_index, true, level);
          }
          break;
          
      case CdlExprOp_Function :
          result = CdlFunction::infer_bool(transaction, expr, index, goal, level);
          break;
              
      default:
          // No other inferences are implemented at this stage.
          break;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlInfer::subexpr_value()                

bool
CdlInfer::subexpr_value(CdlTransaction transaction, CdlExpression expr, unsigned int index, CdlSimpleValue& goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInfer::subexpr_value", "result %d");
    CYG_REPORT_FUNCARG4XV(transaction, expr, index, level);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC((0 <= index) && (index < expr->sub_expressions.size()));
    
    bool result = false;
    CdlSubexpression& subexpr = expr->sub_expressions[index];

    switch(subexpr.op) {
        
      case CdlExprOp_Reference          :
          // The most common case. Follow the reference, and call the appropriate function.
          // Note that the reference may be unbound.
          {
              CdlNode node = expr->references[subexpr.reference_index].get_destination();
              CdlValuable valuable = 0;
              if (0 != node) {
                  valuable = dynamic_cast<CdlValuable>(node);
              }
              result = infer_handle_reference_value(transaction, valuable, goal, level);
              break;
          }
          
      case CdlExprOp_StringConstant     :
          result = infer_handle_string_constant_value(subexpr.constants, goal);
          break;
                                                
      case CdlExprOp_IntegerConstant    :
          result = infer_handle_integer_constant_value(subexpr.constants, goal);
          break;
          
      case CdlExprOp_DoubleConstant     :
          result = infer_handle_double_constant_value(subexpr.constants, goal);
          break;

      case CdlExprOp_LogicalNot         :
      case CdlExprOp_And                :
      case CdlExprOp_Or                 :
      case CdlExprOp_Implies            :
      case CdlExprOp_Xor                :
      case CdlExprOp_Eqv                :
      {
          bool  new_goal = true;
          if (("0" == goal.get_value()) || ("" == goal.get_value())) {
              new_goal = false;
          }
          result = CdlInfer::subexpr_bool(transaction, expr, index, new_goal, level);
          break;
      }
          
      case CdlExprOp_Function :
          result = CdlFunction::infer_value(transaction, expr, index, goal, level);
          break;
        
      default:
          // No other inferences are implemented at this stage.
          break;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}

//}}}
//{{{  Illegal value resolution                 

// ----------------------------------------------------------------------------
// This is not yet implemented.

bool
CdlConflict_IllegalValueBody::inner_resolve(CdlTransaction transaction, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlConflict_IllegalValue::inner_resolve", "result %d");
    CYG_REPORT_FUNCARG3XV(this, transaction, level);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    CYG_UNUSED_PARAM(CdlTransaction, transaction);
    
    CYG_REPORT_RETVAL(false);
    return false;
}

//}}}
//{{{  Requires resolution                      

// ----------------------------------------------------------------------------
// The entry point for this code is
// CdlConflict_RequiresBody::resolve(). "this" is a requires conflict
// that needs to be resolved, if possible. There are twos argument: a
// sub-transaction, which should be filled in with the solution if
// possible; and a recursion level indicator, 0 if this is a top-level
// inference engine invocation rather than a recursive one. There are
// additional static parameters inference_recursion_limit and
// inference_override which control details of the inference process.
//
// As an example of what is involved in an inference, consider the
// simple case of a "requires XXX" property. This constraint may not
// be satisfied because XXX is disabled,  because XXX is inactive,
// or both.
//
// Assume for simplicity that XXX is already active. The inference
// engine can now figure out that XXX must be enabled (it must be
// of type bool or booldata, or else the conflict would not have
// arisen). This is achieved by creating a sub-transaction,
// enabling XXX in that sub-transaction, propagating the
// sub-transaction and performing further inference. The inference
// is successfull if no new conflicts are introduced.
//
// However, even if a solution is found it is not necessarily
// acceptable without user confirmation, subject to
// inference_override. This is handled in part by the transaction
// class itself, in the resolve() and user_confirmation_required()
// members. In cases where the inference engine can choose between
// several alternatives it needs to consider this issue for each one.
// Resolving a requires conflict. There are three ways of tackling
// this problem, in order of preference:
//
// 1) change the terms in the expression to make it evaluate to
//    true.
// 2) disable the source so that the requires property is no longer
//    relevant.
// 3) make the source inactive, with the same effect.
//
// The first one should always be tried. If it is entirely successful
// then there is no point in looking any further. If user confirmation
// is required then the second approach should be tried. If that is
// entirely successful then there is no point in looking further.
// If user confirmation is required then the third approach should
// be tried.

bool
CdlConflict_RequiresBody::inner_resolve(CdlTransaction transaction, int level)
{
    CYG_REPORT_FUNCNAME("CdlConflict_Requires::inner_resolve");
    CYG_REPORT_FUNCARG3XV(this, transaction, level);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    bool result = false;

    CdlProperty_GoalExpression  gexpr   = dynamic_cast<CdlProperty_GoalExpression>(this->get_property());
    CdlExpression               expr    = gexpr->get_expression();
    
    // Only create the sub-transactions when needed.
    CdlTransaction expr_transaction     = 0;
    CdlTransaction disable_transaction  = 0;
    CdlTransaction inactive_transaction = 0;

    // Keep track of the preferred solution found to date.
    CdlTransaction preferred_transaction = 0;

    expr_transaction = transaction->make(this);
    if (!CdlInfer::subexpr_bool(expr_transaction, expr, expr->first_subexpression, true, level)) {
        // No luck here.
        expr_transaction->cancel();
        delete expr_transaction;
        expr_transaction = 0;
    } else {
        // We have a possible solution. How acceptable is it?
        if (!expr_transaction->user_confirmation_required()) {
            // Whoopee.
            expr_transaction->commit();
            delete expr_transaction;
            result = true;
            CYG_REPORT_RETVAL(result);
            return result;
        } else {
            // Maybe we can do better.
            preferred_transaction = expr_transaction;
            expr_transaction = 0;
        }
    }

    // Disabling the source only makes sense if we have a bool or booldata item.
    CdlValuable valuable = dynamic_cast<CdlValuable>(this->get_node());
    CYG_ASSERT_CLASSC(valuable);

    if ((CdlValueFlavor_Bool == valuable->get_flavor()) || (CdlValueFlavor_BoolData == valuable->get_flavor())) {
        disable_transaction = transaction->make(this);
        if (!CdlInfer::set_valuable_bool(disable_transaction, valuable, false, level)) {
            // No luck here either.
            disable_transaction->cancel();
            delete disable_transaction;
            disable_transaction = 0;
        } else {
            if (!disable_transaction->user_confirmation_required()) {
                disable_transaction->commit();
                delete disable_transaction;
                if (0 != preferred_transaction) {
                    preferred_transaction->cancel();
                    delete preferred_transaction;
                    preferred_transaction = 0;
                }
                result = true;
                CYG_REPORT_RETVAL(result);
                return result;
            } else if (0 == preferred_transaction) {
                preferred_transaction = disable_transaction;
            } else if (!preferred_transaction->is_preferable_to(disable_transaction)) {
                preferred_transaction->cancel(); 
                delete preferred_transaction;
                preferred_transaction = disable_transaction;
                disable_transaction = 0;
            } else {
                disable_transaction->cancel();
                delete disable_transaction;
                disable_transaction = 0;
            }
        }
    }

    // Now try for the inactive approach. This may work in cases where the disable
    // approach does not if e.g. there are dependencies between two nodes in the
    // same container, or if the source of the conflict is not boolean.
    inactive_transaction = transaction->make(this);
    if (!CdlInfer::make_inactive(inactive_transaction, valuable, level)) {
        inactive_transaction->cancel();
        delete inactive_transaction;
        inactive_transaction = 0;
    } else {
        if (!inactive_transaction->user_confirmation_required()) {
            inactive_transaction->commit();
            delete inactive_transaction;
            if (0 != preferred_transaction) {
                preferred_transaction->cancel();
                delete preferred_transaction;
                preferred_transaction = 0;
            }
            result = true;
            CYG_REPORT_RETVAL(result);
            return result;
        } else if (0 == preferred_transaction) {
            preferred_transaction = inactive_transaction;
        } else if (!preferred_transaction->is_preferable_to(inactive_transaction)) {
            preferred_transaction->cancel(); 
            delete preferred_transaction;
            preferred_transaction = inactive_transaction;
            inactive_transaction = 0;
        } else {
            inactive_transaction->cancel();
            delete inactive_transaction;
            inactive_transaction = 0;
        }
    }

    // Is there any solution at all? If so then use the currently-preferred one.
    if (0 != preferred_transaction) {
        preferred_transaction->commit();
        delete preferred_transaction;
        preferred_transaction = 0;
        result = true;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
