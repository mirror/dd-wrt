//{{{  Banner                           

//============================================================================
//
//      func.cxx
//
//      Implementation of CDL functions
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2001 Red Hat, Inc.
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
// Date:        2001/04/20
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

//{{{  Core                             

// ----------------------------------------------------------------------------
int CdlFunction::next_id        = 1;
std::vector<CdlFunction*>       CdlFunction::all_functions;

// Dummy initializers, for e.g. when a particular function implementation does not
// support a certain type of inference.
void (*CdlFunction::null_check)(CdlExpression, const CdlSubexpression&) =
     (void (*)(CdlExpression, const CdlSubexpression&)) 0;
bool (*CdlFunction::null_infer_bool)(CdlTransaction, CdlExpression, unsigned int, bool, int) =
     (bool (*)(CdlTransaction, CdlExpression, unsigned int, bool, int)) 0;
bool (*CdlFunction::null_infer_value)(CdlTransaction, CdlExpression, unsigned int, CdlSimpleValue&, int) =
     (bool (*)(CdlTransaction, CdlExpression, unsigned int, CdlSimpleValue&, int)) 0;


CdlFunction::CdlFunction(const char* name_arg, int number_args_arg,
                         void (*check_arg)(CdlExpression, const CdlSubexpression&),
                         void (*eval_arg)(CdlEvalContext&, CdlExpression, const CdlSubexpression&, CdlSimpleValue&),
                         bool (*infer_bool_arg)(CdlTransaction, CdlExpression, unsigned int, bool, int),
                         bool (*infer_value_arg)(CdlTransaction, CdlExpression, unsigned int, CdlSimpleValue&, int))
    : name(name_arg),
      number_args(number_args_arg),
      check_fn(check_arg),
      eval_fn(eval_arg),
      infer_bool_fn(infer_bool_arg),
      infer_value_fn(infer_value_arg)
{
    id  = next_id++;
    all_functions.push_back(this);
}

CdlFunction::~CdlFunction()
{
}

bool
CdlFunction::is_function(std::string name, int& id)
{
    CYG_REPORT_FUNCNAMETYPE("CdlFunction::is_function", "result %d");
    
    bool result = false;
    std::vector<CdlFunction*>::const_iterator i;
    
    for (i = all_functions.begin(); !result && (i != all_functions.end()); i++) {
        if (name == (*i)->name) {
            result = true;
            id = (*i)->id;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlFunction::get_name(int id)
{
    CYG_REPORT_FUNCNAME("CdlFunction::get_name");
    CYG_REPORT_FUNCARG1XV(id);
    
    std::string result  = "";
    std::vector<CdlFunction*>::const_iterator i;
    
    for (i = all_functions.begin(); i != all_functions.end(); i++) {
        if (id == (*i)->id) {
            result = (*i)->name;
            break;
        }
    }

    CYG_REPORT_RETURN();
    return result;
}

int
CdlFunction::get_args_count(int id)
{
    CYG_REPORT_FUNCNAMETYPE("CdlFunction::get_args_count", "result %d");
    CYG_REPORT_FUNCARG1XV(id);
    
    int result = 0;
    std::vector<CdlFunction*>::const_iterator i;
    
    for (i = all_functions.begin(); i != all_functions.end(); i++) {
        if (id == (*i)->id) {
            result = (*i)->number_args;;
            break;
        }
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlFunction::check(CdlExpression expr, const CdlSubexpression& subexpr)
{
    CYG_REPORT_FUNCNAME("CdlFunction::check");
    CYG_REPORT_FUNCARG2XV(expr, &subexpr);
    
    int id = subexpr.func;
    std::vector<CdlFunction*>::const_iterator i;
    
    for (i = all_functions.begin(); i != all_functions.end(); i++) {
        if (id == (*i)->id) {
            if (CdlFunction::null_check != (*i)->check_fn) {
                (*((*i)->check_fn))(expr, subexpr);
            }
            break;
        }
    }

    CYG_REPORT_RETURN();
}

void
CdlFunction::eval(CdlEvalContext& context, CdlExpression expr, const CdlSubexpression& subexpr, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("CdlFunction::eval");
    CYG_REPORT_FUNCARG4XV(&context, expr, &subexpr, &result);
    
    int id = subexpr.func;
    std::vector<CdlFunction*>::const_iterator i;
    
    for (i = all_functions.begin(); i != all_functions.end(); i++) {
        if (id == (*i)->id) {
            (*((*i)->eval_fn))(context, expr, subexpr, result);
            break;
        }
    }

    CYG_REPORT_RETURN();
}

bool
CdlFunction::infer_bool(CdlTransaction transaction, CdlExpression expr, unsigned int index, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlFunction::infer_bool", "result %d");
    CYG_REPORT_FUNCARG5XV(transaction, expr, index, goal, level);

    bool result = false;
    CdlSubexpression& subexpr = expr->sub_expressions[index];
    int id = subexpr.func;
    std::vector<CdlFunction*>::const_iterator i;

    for (i = all_functions.begin(); i != all_functions.end(); i++) {
        if (id == (*i)->id) {
            if (CdlFunction::null_infer_bool != (*i)->infer_bool_fn) {
                result = (*((*i)->infer_bool_fn))(transaction, expr, index, goal, level);
            }
            break;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlFunction::infer_value(CdlTransaction transaction, CdlExpression expr, unsigned int index, CdlSimpleValue& goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("CdlFunction::infer_value", "result %d");
    CYG_REPORT_FUNCARG5XV(transaction, expr, index, &goal, level);
    
    bool result = false;
    CdlSubexpression& subexpr = expr->sub_expressions[index];
    int id = subexpr.func;
    std::vector<CdlFunction*>::const_iterator i;

    for (i = all_functions.begin(); i != all_functions.end(); i++) {
        if (id == (*i)->id) {
            if (CdlFunction::null_infer_value != (*i)->infer_value_fn) {
                result = (*((*i)->infer_value_fn))(transaction, expr, index, goal, level);
            }
            break;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  is_substr()                      

// ----------------------------------------------------------------------------
// is_substr(A, B)
//
// For example, is_substr(CYGBLD_GLOBAL_CFLAGS, " -fno-exceptions ")
//
// There is one subtlety about substring matching: what to do about the
// start and end of a string. If the specified substring begins with a
// space then this will match either a space or the start of the string,
// similarly for the final character.

static std::string::size_type
is_substr_find(std::string haystack, std::string needle, std::string::size_type& len_arg)
{
    CYG_REPORT_FUNCNAMETYPE("is_substr_find", "result %d");
    std::string::size_type result       = std::string::npos;

    std::string::size_type haystack_len = haystack.length();
    std::string::size_type needle_len   = needle.length();

    bool leading_space = false;
    bool trailing_space = false;
    
    if (' ' == needle[0]) {
        leading_space = true;
        needle_len--;
        needle = std::string(needle, 1, needle_len);
    }
    if (' ' == needle[needle_len - 1]) {
        trailing_space = true;
        needle_len--;
        needle = std::string(needle, 0, needle_len);
    }

    std::string::size_type posn = haystack.find(needle);
    while ((std::string::npos == result) && (std::string::npos != posn)) {

        std::string::size_type match_point = posn;
        bool match = true;

        // A possible match has been found. If there was a leading
        // space, check we are either at the start of the main string
        // or that a space is present.
        if (leading_space && (0 != posn) && (' ' != haystack[posn - 1])) {
            match = false;
        }
        if (trailing_space && (haystack_len != (posn + needle_len)) && (' ' != haystack[posn + needle_len])) {
            match = false;
        }

        // The result and len_arg returns exclude the spaces. This is deliberate.
        // Consider !is_substr("-g -O2 -fno-rtti -fno-exceptions", " -fnortti ").
        // If during inference the spaces were removed as well, this would give
        // "-g -O2-fno-exceptions", which is not desirable.
        if (match) {
            result  = match_point;
            len_arg = needle_len;
        } else {
            posn = haystack.find(needle, posn + 1);
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

static void
is_substr_eval(CdlEvalContext& context, CdlExpression expr, const CdlSubexpression& subexpr, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("is_substr_eval");
    CYG_REPORT_FUNCARG4XV(&context, expr, &subexpr, &result);
    CYG_PRECONDITION_CLASSOC(context);
    CYG_PRECONDITION_CLASSC(expr);

    CdlSimpleValue      arg0;
    CdlSimpleValue      arg1;
    expr->eval_subexpression(context, subexpr.args[0], arg0);
    expr->eval_subexpression(context, subexpr.args[1], arg1);

    std::string::size_type len;
    result = (std::string::npos != is_substr_find(arg0.get_value(), arg1.get_value(), len));
    CYG_REPORT_RETURN();
}

// Inference is only supported if the haystack argument is a reference that can be
// updated. The needle can be an arbitrary expression.
static bool
is_substr_infer_bool(CdlTransaction transaction, CdlExpression expr, unsigned int index, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("is_substr_infer_bool", "result %d");
    CYG_REPORT_FUNCARG5XV(transaction, expr, index, goal, level);

    bool result = false;

    CdlSubexpression& subexpr   = expr->sub_expressions[index];
    CdlSubexpression& arg0      = expr->sub_expressions[subexpr.args[0]];
    try {
        if (CdlExprOp_Reference == arg0.op) {

            CdlSimpleValue  needle_value;
            CdlEvalContext context(transaction);
            expr->eval_subexpression(context, subexpr.args[1], needle_value);
            std::string     needle  = needle_value.get_value();

            CdlNode         node        = expr->references[arg0.reference_index].get_destination();
            CdlValuable     valuable    = 0;
            if (0 != node) {
                valuable = dynamic_cast<CdlValuable>(node);
            }
            if ((0 != valuable) && ((CdlValueFlavor_BoolData == valuable->get_flavor()) ||
                                    (CdlValueFlavor_Data == valuable->get_flavor()))) {
                // OK, we have a valuable which can be given a suitable value.
                // What is the current string?
                const CdlValue& current_value   = transaction->get_whole_value(valuable);
                std::string haystack            = current_value.get_simple_value().get_value();

                // What is the goal? If the needle should be in the
                // haystack, append it if necessary. If the needle
                // should not be in the haystack, remove all current occurrences.
                if (goal) {
                    std::string::size_type len;
                    if (std::string::npos == is_substr_find(haystack, needle, len)) {
                        haystack = haystack + needle;
                    }
                } else {
                    std::string::size_type posn, len;
                    for (posn = is_substr_find(haystack, needle, len);
                         std::string::npos != posn;
                         posn = is_substr_find(haystack, needle, len)) {
                        haystack.erase(posn, len);
                    }
                }

                // OK, we have a new value for the haystack which should match the desired goal.
                // Try and set this value.
                CdlSimpleValue  new_value(haystack);
                result = CdlInfer::set_valuable_value(transaction, valuable, new_value, level);
            }
        }
    } catch (...) {
        result = false;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static CdlFunction is_substr("is_substr", 2, CdlFunction::null_check, &is_substr_eval,
                             &is_substr_infer_bool, CdlFunction::null_infer_value);

//}}}
//{{{  is_xsubstr()                     

// ----------------------------------------------------------------------------
// is_xsubstr(A, B)
//
// Like is_substr() but only deals with exact matches, i.e. there is no special
// treatment for leading and trailing spaces in the needle.

static void
is_xsubstr_eval(CdlEvalContext& context, CdlExpression expr, const CdlSubexpression& subexpr, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("is_xsubstr_eval");
    CYG_REPORT_FUNCARG4XV(&context, expr, &subexpr, &result);
    CYG_PRECONDITION_CLASSOC(context);
    CYG_PRECONDITION_CLASSC(expr);

    CdlSimpleValue      arg0;
    CdlSimpleValue      arg1;
    expr->eval_subexpression(context, subexpr.args[0], arg0);
    expr->eval_subexpression(context, subexpr.args[1], arg1);

    result = (std::string::npos != arg0.get_value().find(arg1.get_value()));
    CYG_REPORT_RETURN();
}

// Inference is only supported if the haystack argument is a reference that can be
// updated. The needle can be an arbitrary expression.
static bool
is_xsubstr_infer_bool(CdlTransaction transaction, CdlExpression expr, unsigned int index, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("is_xsubstr_infer_bool", "result %d");
    CYG_REPORT_FUNCARG5XV(transaction, expr, index, goal, level);

    bool result = false;

    CdlSubexpression& subexpr   = expr->sub_expressions[index];
    CdlSubexpression& arg0      = expr->sub_expressions[subexpr.args[0]];
    try {
        if (CdlExprOp_Reference == arg0.op) {

            CdlSimpleValue  needle_value;
            CdlEvalContext context(transaction);
            expr->eval_subexpression(context, subexpr.args[1], needle_value);
            std::string     needle  = needle_value.get_value();

            CdlNode         node        = expr->references[arg0.reference_index].get_destination();
            CdlValuable     valuable    = 0;
            if (0 != node) {
                valuable = dynamic_cast<CdlValuable>(node);
            }
            if ((0 != valuable) && ((CdlValueFlavor_BoolData == valuable->get_flavor()) ||
                                    (CdlValueFlavor_Data == valuable->get_flavor()))) {
                // OK, we have a valuable which can be given a suitable value.
                // What is the current string?
                const CdlValue& current_value   = transaction->get_whole_value(valuable);
                std::string haystack            = current_value.get_simple_value().get_value();

                // What is the goal? If the needle should be in the
                // haystack, append it if necessary. If the needle
                // should not be in the haystack, remove all current occurrences.
                if (goal) {
                    if (std::string::npos == haystack.find(needle)) {
                        haystack = haystack + needle;
                    }
                } else {
                    std::string::size_type posn;
                    for (posn = haystack.find(needle); std::string::npos != posn; posn = haystack.find(needle)) {
                        haystack.erase(posn, needle.length());
                    }
                }

                // OK, we have a new value for the haystack which should match the desired goal.
                // Try and set this value.
                CdlSimpleValue  new_value(haystack);
                result = CdlInfer::set_valuable_value(transaction, valuable, new_value, level);
            }
        }
    } catch (...) {
        result = false;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static CdlFunction is_xsubstr("is_xsubstr", 2, CdlFunction::null_check, &is_xsubstr_eval,
                              &is_xsubstr_infer_bool, CdlFunction::null_infer_value);

//}}}
//{{{  is_loaded()                      

// ----------------------------------------------------------------------------
// is_loaded(x)
// Check whether or not a particular configuration option is loaded.
// This takes a single argument which must be a reference. No
// inference is possible, since loading and unloading packages is
// currently beyond the scope of the inference engine.

static void
is_loaded_check(CdlExpression expr, const CdlSubexpression& subexpr)
{
    CYG_REPORT_FUNCNAME("is_loaded_check");
    CYG_REPORT_FUNCARG2XV(expr, &subexpr);

    CdlSubexpression& arg0 = expr->sub_expressions[subexpr.args[0]];
    if (CdlExprOp_Reference != arg0.op) {
        throw CdlParseException(std::string("The argument to is_loaded() should be a reference to a configuration option.\n") +
                                CdlParse::get_expression_error_location());
    }
    
    CYG_REPORT_RETURN();
}

static void
is_loaded_eval(CdlEvalContext& context, CdlExpression expr, const CdlSubexpression& subexpr, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("is_loaded_eval");
    CYG_REPORT_FUNCARG4XV(&context, expr, &subexpr, &result);
    CYG_PRECONDITION_CLASSOC(context);
    CYG_PRECONDITION_CLASSC(expr);

    CdlSubexpression arg0 = expr->sub_expressions[subexpr.args[0]];
    CYG_ASSERTC(CdlExprOp_Reference == arg0.op);

    result = (0 != context.resolve_reference(expr, arg0.reference_index));
    CYG_REPORT_RETURN();
}

static CdlFunction is_loaded("is_loaded", 1, &is_loaded_check, &is_loaded_eval,
                             CdlFunction::null_infer_bool, CdlFunction::null_infer_value);

//}}}
//{{{  is_active()                      

// ----------------------------------------------------------------------------
// is_active(x)
// Check whether or not a particular configuration option is loaded
// and active. This takes a single argument which must be a reference.

static void
is_active_check(CdlExpression expr, const CdlSubexpression& subexpr)
{
    CYG_REPORT_FUNCNAME("is_active_check");
    CYG_REPORT_FUNCARG2XV(expr, &subexpr);

    CdlSubexpression& arg0 = expr->sub_expressions[subexpr.args[0]];
    if (CdlExprOp_Reference != arg0.op) {
        throw CdlParseException(std::string("The argument to is_active() should be a reference to a configuration option.\n") +
                                CdlParse::get_expression_error_location());
    }
    
    CYG_REPORT_RETURN();
}

static void
is_active_eval(CdlEvalContext& context, CdlExpression expr, const CdlSubexpression& subexpr, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("is_active_eval");
    CYG_REPORT_FUNCARG4XV(&context, expr, &subexpr, &result);
    CYG_PRECONDITION_CLASSOC(context);
    CYG_PRECONDITION_CLASSC(expr);

    CdlSubexpression arg0 = expr->sub_expressions[subexpr.args[0]];
    CYG_ASSERTC(CdlExprOp_Reference == arg0.op);

    CdlNode node = context.resolve_reference(expr, arg0.reference_index);
    if (0 != node) {
        result = node->is_active(context.transaction);
    } else {
        result = false;
    }
    CYG_REPORT_RETURN();
}

static bool
is_active_infer_bool(CdlTransaction transaction, CdlExpression expr, unsigned int index, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("is_active_infer_bool", "result %d");
    CYG_REPORT_FUNCARG5XV(transaction, expr, index, goal, level);

    bool result = false;

    CdlSubexpression subexpr    = expr->sub_expressions[index];
    CdlSubexpression arg0       = expr->sub_expressions[subexpr.args[0]];
    CYG_ASSERTC(CdlExprOp_Reference == arg0.op);

    CdlNode node = expr->references[arg0.reference_index].get_destination();
    if (0 != node) {
        if (goal) {
            result = CdlInfer::make_active(transaction, node, level);
        } else {
            result = CdlInfer::make_inactive(transaction, node, level);
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static CdlFunction is_active("is_active", 1, &is_active_check, &is_active_eval,
                             &is_active_infer_bool, CdlFunction::null_infer_value);

//}}}
//{{{  is_enabled()                     

// ----------------------------------------------------------------------------
// is_enabled(x)
// Check whether or not a particular configuration option is loaded
// and enabled. The active/inactive state is ignored. This function
// takes a single argument which must be a reference.

static void
is_enabled_check(CdlExpression expr, const CdlSubexpression& subexpr)
{
    CYG_REPORT_FUNCNAME("is_enabled_check");
    CYG_REPORT_FUNCARG2XV(expr, &subexpr);

    CdlSubexpression& arg0 = expr->sub_expressions[subexpr.args[0]];
    if (CdlExprOp_Reference != arg0.op) {
        throw CdlParseException(std::string("The argument to is_enabled() should be a reference to a configuration option.\n") +
                                CdlParse::get_expression_error_location());
    }
    
    CYG_REPORT_RETURN();
}

static void
is_enabled_eval(CdlEvalContext& context, CdlExpression expr, const CdlSubexpression& subexpr, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("is_enabled_eval");
    CYG_REPORT_FUNCARG4XV(&context, expr, &subexpr, &result);
    CYG_PRECONDITION_CLASSOC(context);
    CYG_PRECONDITION_CLASSC(expr);

    CdlSubexpression arg0 = expr->sub_expressions[subexpr.args[0]];
    CYG_ASSERTC(CdlExprOp_Reference == arg0.op);

    CdlValuable valuable = context.resolve_valuable_reference(expr, arg0.reference_index);
    if (0 != valuable) {
        if (0 != context.transaction) {
            result = valuable->is_enabled(context.transaction);
        } else {
            result = valuable->is_enabled();
        }
    } else {
        result = false;
    }
    CYG_REPORT_RETURN();
}

static bool
is_enabled_infer_bool(CdlTransaction transaction, CdlExpression expr, unsigned int index, bool goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("is_enabled_infer_bool", "result %d");
    CYG_REPORT_FUNCARG5XV(transaction, expr, index, goal, level);

    bool result = false;

    CdlSubexpression subexpr    = expr->sub_expressions[index];
    CdlSubexpression arg0       = expr->sub_expressions[subexpr.args[0]];
    CYG_ASSERTC(CdlExprOp_Reference == arg0.op);

    CdlNode node = expr->references[arg0.reference_index].get_destination();
    if (0 != node) {
        CdlValuable valuable    = dynamic_cast<CdlValuable>(node);
        if (0 != valuable) {
            // OK, we have found a valuable. Is it already enabled?
            // Does it have a boolean component? Is it modifiable? Has
            // it already been modified by the user in this transaction?
            if (goal == valuable->is_enabled()) {
                result = true;
            } else {
                CdlValueFlavor flavor = valuable->get_flavor();
                if ((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor)) {
                    if (valuable->is_modifiable()) {
                        if (!transaction->changed_by_user(valuable)) {
                            // We have a modifiable option and want to set the enabled flag.
                            // However we do not want to lose the current data part - unless
                            // some other constraint has caused that to be set.
                            const CdlValue& old_value   = transaction->get_whole_value(valuable);
                            CdlValue        new_value   = old_value;
                            if (!old_value.has_source(CdlValueSource_Inferred)) {
                                CdlSimpleValue simple_value = old_value.get_simple_value(CdlValueSource_Current);
                                new_value.set_value(simple_value, CdlValueSource_Inferred);
                            }
                            new_value.set_enabled(goal, CdlValueSource_Inferred);
                            new_value.set_source(CdlValueSource_Inferred);
                            transaction->set_whole_value(valuable, old_value, new_value);
                            result = transaction->resolve_recursion(level);
                        }
                    }
                }
            }
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static CdlFunction is_enabled("is_enabled", 1, &is_enabled_check, &is_enabled_eval,
                              &is_enabled_infer_bool, CdlFunction::null_infer_value);

//}}}
//{{{  get_data()                       

// ----------------------------------------------------------------------------
// get_data(x)
// Returns "0" if the specified option is not enabled, otherwise
// the current data part fo the value. The active/inactive and the
// enabled states are ignored. This function takes a single argument
// which must be a reference.

static void
get_data_check(CdlExpression expr, const CdlSubexpression& subexpr)
{
    CYG_REPORT_FUNCNAME("get_data_check");
    CYG_REPORT_FUNCARG2XV(expr, &subexpr);

    CdlSubexpression& arg0 = expr->sub_expressions[subexpr.args[0]];
    if (CdlExprOp_Reference != arg0.op) {
        throw CdlParseException(std::string("The argument to get_data() should be a reference to a configuration option.\n") +
                                CdlParse::get_expression_error_location());
    }
    
    CYG_REPORT_RETURN();
}

static void
get_data_eval(CdlEvalContext& context, CdlExpression expr, const CdlSubexpression& subexpr, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("get_data_eval");
    CYG_REPORT_FUNCARG4XV(&context, expr, &subexpr, &result);
    CYG_PRECONDITION_CLASSOC(context);
    CYG_PRECONDITION_CLASSC(expr);

    CdlSubexpression arg0 = expr->sub_expressions[subexpr.args[0]];
    CYG_ASSERTC(CdlExprOp_Reference == arg0.op);

    CdlValuable valuable = context.resolve_valuable_reference(expr, arg0.reference_index);
    if (0 != valuable) {
        if (0 != context.transaction) {
            result = valuable->get_value(context.transaction);
        } else {
            result = valuable->get_value();
        }
    } else {
        result = false;
    }
    CYG_REPORT_RETURN();
}

static bool
get_data_infer_value(CdlTransaction transaction, CdlExpression expr, unsigned int index,  CdlSimpleValue& goal, int level)
{
    CYG_REPORT_FUNCNAMETYPE("get_data_infer_value", "result %d");
    CYG_REPORT_FUNCARG5XV(transaction, expr, index, &goal, level);

    bool result = false;

    CdlSubexpression subexpr    = expr->sub_expressions[index];
    CdlSubexpression arg0       = expr->sub_expressions[subexpr.args[0]];
    CYG_ASSERTC(CdlExprOp_Reference == arg0.op);

    CdlNode node = expr->references[arg0.reference_index].get_destination();
    if (0 != node) {
        CdlValuable valuable    = dynamic_cast<CdlValuable>(node);
        if (0 != valuable) {
            // OK, we have found a valuable. Does it have a data component?
            // Does it already have the right value. Is it modifiable? Has
            // it already been modified by the user in this transaction?
            CdlValueFlavor flavor = valuable->get_flavor();
            if ((CdlValueFlavor_Data == flavor) || (CdlValueFlavor_BoolData == flavor)) {
                CdlSimpleValue current_value = valuable->get_simple_value(transaction);
                if (goal != current_value) {
                    if (valuable->is_modifiable()) {
                        if (!transaction->changed_by_user(valuable)) {
                            // We have a modifiable option and want to set the data part.
                            // However we do not want to lose the enabled part - unless
                            // some other constraint has caused that to be set.
                            const CdlValue& old_value   = transaction->get_whole_value(valuable);
                            CdlValue        new_value   = old_value;
                            if (!old_value.has_source(CdlValueSource_Inferred)) {
                                new_value.set_enabled(old_value.is_enabled(), CdlValueSource_Inferred);
                            }
                            new_value.set_value(goal, CdlValueSource_Inferred);
                            new_value.set_source(CdlValueSource_Inferred);
                            transaction->set_whole_value(valuable, old_value, new_value);
                            result = transaction->resolve_recursion(level);
                        }
                    }
                }
            }
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static CdlFunction get_data("get_data", 1, &get_data_check, &get_data_eval,
                            CdlFunction::null_infer_bool, &get_data_infer_value);

//}}}
//{{{  version_cmp()                    

// ----------------------------------------------------------------------------
// version_cmp(a, b)
// Evaluate both arguments, interpret them as version strings, and then
// return -1, 0 or 1.

static void
version_cmp_eval(CdlEvalContext& context, CdlExpression expr, const CdlSubexpression& subexpr, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("version_cmp_eval");
    CYG_REPORT_FUNCARG4XV(&context, expr, &subexpr, &result);
    CYG_PRECONDITION_CLASSOC(context);
    CYG_PRECONDITION_CLASSC(expr);

    CdlSimpleValue      arg0;
    CdlSimpleValue      arg1;
    expr->eval_subexpression(context, subexpr.args[0], arg0);
    expr->eval_subexpression(context, subexpr.args[1], arg1);

    result = (cdl_int) Cdl::compare_versions(arg0.get_value(), arg1.get_value());
    
    CYG_REPORT_RETURN();
}

static CdlFunction version_cmp("version_cmp", 2, CdlFunction::null_check, &version_cmp_eval,
                               CdlFunction::null_infer_bool, CdlFunction::null_infer_value);

//}}}
