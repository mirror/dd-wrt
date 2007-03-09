//{{{  Banner                           

//============================================================================
//
//      expr.cxx
//
//      Implementation of the various CDL expression classes.
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
// Date:        1999/02/02
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
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlEvalContext);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlExpressionBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlListExpressionBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlGoalExpressionBody);

//}}}
//{{{  CdlEvalContext                   

// ----------------------------------------------------------------------------
// A utility class to keep track of the context in which expression
// evaluation is happening.

CdlEvalContext::CdlEvalContext(CdlTransaction transaction_arg, CdlNode node_arg, CdlProperty property_arg,
                               CdlToplevel toplevel_arg)
{
    CYG_REPORT_FUNCNAME("CdlEvalContext::constructor");
    CYG_REPORT_FUNCARG4XV(this, transaction_arg, node_arg, property_arg);

    transaction = transaction_arg;
    
    if ((0 == property_arg) && (0 != transaction)) {
        CdlConflict conflict = transaction->get_conflict();
        if (0 != conflict) {
            property_arg = conflict->get_property();
        }
    }
    property    = property_arg;
    
    if ((0 == node_arg) && (0 != transaction)) {
        CdlConflict conflict = transaction->get_conflict();
        if (0 != conflict) {
            node_arg = conflict->get_node();
        }
    }
    node        = node_arg;
    
    if (0 == toplevel_arg) {
        if (0 != transaction) {
            toplevel_arg = transaction->get_toplevel();
        } else if (0 != node) {
            toplevel_arg = node->get_toplevel();
        }
    }
    toplevel = toplevel_arg;
    
    cdlevalcontext_cookie = CdlEvalContext_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlEvalContext::~CdlEvalContext()
{
    CYG_REPORT_FUNCNAME("CdlEvalContext::destructor");
    CYG_PRECONDITION_THISC();

    cdlevalcontext_cookie       = CdlEvalContext_Invalid;
    transaction = 0;
    node        = 0;
    property    = 0;
    toplevel    = 0;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

// Given a context and a reference inside an expression, obtain the node
// being referenced - if it is loaded.
CdlNode
CdlEvalContext::resolve_reference(CdlExpression expr, int index)
{
    CYG_REPORT_FUNCNAMETYPE("CdlEvalContext::resolve_reference", "result %");
    CYG_REPORT_FUNCARG2XV(expr, index);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC((0 <= index) && (index <= (int)expr->references.size()));

    // This expression may be happening in the context of a particular
    // property. If so then the destination may or may not be
    // resolved, which will have been handled when the containing package
    // was loaded. Alternatively this expression may be evaluated inside
    // some arbitrary Tcl code, in which case references remain unbound
    // and need to be resolved the hard way.
    CdlNode result = 0;
    if (0 != this->property) {
        // There is a property, use the bound/unbound reference.
        result = expr->references[index].get_destination();
    } else {
        // The destination name can be retrieved, but we still need some
        // way of resolving it.
        if (0 != this->toplevel) {
            std::string destination_name = expr->references[index].get_destination_name();
            result = this->toplevel->lookup(destination_name);
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// Ditto, but also check that the result is a valuable.
CdlValuable
CdlEvalContext::resolve_valuable_reference(CdlExpression expr, int index)
{
    CYG_REPORT_FUNCNAMETYPE("CdlEvalContext::resolve_reference", "result %");
    CYG_REPORT_FUNCARG2XV(expr, index);

    CdlValuable result  = 0;
    CdlNode     node = this->resolve_reference(expr, index);
    if (0 != node) {
        result = dynamic_cast<CdlValuable>(node);
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlEvalContext::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlEvalContext_Magic != cdlevalcontext_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    
    if ((0 != transaction) && !transaction->check_this(zeal)) {
        return false;
    }
    if ((0 != toplevel) && !toplevel->check_this(zeal)) {
        return false;
    }
    if ((0 != node) && !node->check_this(zeal)) {
        return false;
    }
    if ((0 != property) && !property->check_this(zeal)) {
        return false;
    }
    return true;
}

//}}}
//{{{  Expression parsing               

//{{{  Description                      

// ----------------------------------------------------------------------------
// There are a number of different entry points related to expression parsing,
// largely to support list and goal expressions. All of these eventually
// end up calling the function
//    continue_parse(expr, data, index, token, token_end)
//
// The expr argument holds an existing expression object that needs to be
// updated. If token is Invalid then we are at the start of an expression
// (but not necessarily at the start of the string).
//
// The data string holds all of the expression that should be parsed.
// It is formed by concatenating all non-option arguments to the
// appropriate property command, with spaces between them.
//
// index is an input/output variable. On input it indicates where in
// the string parsing should continue. On output it indicates the
// location within the string where the terminating token began.
//
// token is an input/output variable. On input it can have the values
// Invalid or And. The former means that we are parsing a completely
// new expression. The latter is used for goal expressions: it is
// necessary to parse a new expression and then combine it with the
// existing one.
//
// token_end is an output variable. It indicates the location within
// the string where the terminating token ended. This is useful for
// e.g. ranges in a list expression.
//
// A conventional recursive descent parser is used.

//}}}
//{{{  Tokenization                     

// ----------------------------------------------------------------------------
// Tokenization.

//{{{  token enum                       

// A separate token enum is necessary, rather than re-using the CdlExprOp
// enum. Some tokens may correspond to several operators, and some tokens
// such as close-bracket do not correspond directly to an operator at all.
enum token {
    T_Invalid           = -2,

    T_EOD               = -1,
    T_Reference         =  1,   // CYGPKG_HAL
    T_String            =  2,   // "hello"
    T_Integer           =  3,   // 123
    T_Double            =  4,   // 3.1415
    T_Range             =  5,   // to
    T_OpenBracket       =  6,   // (
    T_CloseBracket      =  7,   // )
    T_Minus             =  8,   // -
    T_Plus              =  9,   // +
    T_Times             = 10,   // *
    T_Divide            = 11,   // /
    T_Exclamation       = 12,   // !
    T_Tilde             = 13,   // ~
    T_Questionmark      = 14,   // ?
    T_Remainder         = 15,   // %
    T_LeftShift         = 16,   // <<
    T_RightShift        = 17,   // >>
    T_LessThan          = 18,   // <
    T_LessEqual         = 19,   // <=
    T_GreaterThan       = 20,   // >
    T_GreaterEqual      = 21,   // >=
    T_Equal             = 22,   // ==
    T_NotEqual          = 23,   // !=
    T_BitAnd            = 24,   // &
    T_BitXor            = 25,   // ^
    T_BitOr             = 26,   // |
    T_And               = 27,   // &&
    T_Or                = 28,   // ||
    T_Colon             = 29,   // : (in a conditional)
    T_StringConcat      = 30,   // .
    T_Function          = 31,   // is_substr etc.
    T_Comma             = 32,   // , (inside a function)
    T_Implies           = 33,   // implies
    T_Xor               = 34,   // xor
    T_Eqv               = 35    // eqv
    
};

//}}}
//{{{  Statics                          

// Statics to keep track of the current state.
static std::string      current_data            = "";
static unsigned int     current_index           = 0;
static unsigned int     token_start             = 0;
static int              current_char            = EOF;
static token            current_token           = T_Invalid;
static std::string      current_string          = "";
static std::string      current_reference       = "";
static std::string      current_special         = "";
static cdl_int          current_int             = 0;
static double           current_double          = 0.0;
static CdlValueFormat   current_format          = CdlValueFormat_Default;
static int              current_function_id     = 0;

//}}}
//{{{  Character access                 

// ----------------------------------------------------------------------------
// Individual character access.
// Note that current_index is one character past current_char.

// Return the next character in the string, or EOF
static void
next_char()
{
    if (current_index >= current_data.size()) {
        current_char = EOF;
    } else {
        current_char = current_data[current_index++];
    }
}

// Go back a character. This is useful when parsing
// strings. It is the responsibility of the calling code
// to make sure that we are not at the start of the buffer.
static void
backup_char()
{
    CYG_ASSERTC(((EOF == current_char) && (0 < current_index)) || (1 < current_index));
    if (EOF != current_char) {
        current_index--;
    }
    current_char = current_data[current_index - 1];
}

//}}}
//{{{  get_error_location()             

// ----------------------------------------------------------------------------
// Construct part of a diagnostic message, indicating the
// area in the data where the error occurred. This string
// is of the form {...data} ^char^ {data...}. Ideally
// the ^ markers would be on a subsequent line, eliminating
// the need for braces, but there is insufficient control
// of how the message gets presented to the user.
//
// Care has to be taken with EOD.
static std::string
get_error_location()
{
    CYG_REPORT_FUNCNAME("get_error_location");
    std::string result = "";

    // token_start is probably the best place for centering the error.
    // current_index is past the point where the error has occurred.
    if (token_start > 1) {
        if (token_start > 16) {
            result = "{..." + current_data.substr(token_start - 13, 13) + "} ";
        } else {
            result = "{" + current_data.substr(0, token_start) + "}";
        }
    }

    if (current_char == EOF) {
        result += " <end of data>";
    } else {
        result += " ^" + std::string(1, current_data[token_start]) + "^ ";
    }

    if (token_start < current_data.size()) {
        if ((token_start + 16) < current_data.size()) {
            result += "{" + current_data.substr(token_start + 1, current_data.size() - (token_start+1)) + "}";
        } else {
            result += "{" + current_data.substr(token_start, 13) + "...}";
        }
    }

    CYG_REPORT_RETURN();
    return result;
}

// Export this functionality available to other modules, especially func.cxx and its
// argument checking routines.
std::string
CdlParse::get_expression_error_location(void)
{
    return get_error_location();
}

//}}}
//{{{  Token translation                

// ----------------------------------------------------------------------------

// Convert a token into a binary expression operator
static CdlExprOp
token_to_binary_expr_op()
{
    CYG_REPORT_FUNCNAMETYPE("token_to_expr_op", "op %d");
    CdlExprOp result = CdlExprOp_Invalid;

    switch(current_token) {
      case T_Minus:             result = CdlExprOp_Subtract; break;
      case T_Plus:              result = CdlExprOp_Add; break;
      case T_Times:             result = CdlExprOp_Multiply; break;
      case T_Divide:            result = CdlExprOp_Divide; break;
      case T_Remainder:         result = CdlExprOp_Remainder; break;
      case T_LeftShift:         result = CdlExprOp_LeftShift; break;
      case T_RightShift:        result = CdlExprOp_RightShift; break;
      case T_LessThan:          result = CdlExprOp_LessThan; break;
      case T_LessEqual:         result = CdlExprOp_LessEqual; break;
      case T_GreaterThan:       result = CdlExprOp_GreaterThan; break;
      case T_GreaterEqual:      result = CdlExprOp_GreaterEqual; break;
      case T_Equal:             result = CdlExprOp_Equal; break;
      case T_NotEqual:          result = CdlExprOp_NotEqual; break;
      case T_BitAnd:            result = CdlExprOp_BitAnd; break;
      case T_BitXor:            result = CdlExprOp_BitXor; break;
      case T_BitOr:             result = CdlExprOp_BitOr; break;
      case T_And:               result = CdlExprOp_And; break;
      case T_Or:                result = CdlExprOp_Or; break;
      case T_StringConcat:      result = CdlExprOp_StringConcat; break;
      case T_Implies:           result = CdlExprOp_Implies; break;
      case T_Xor:               result = CdlExprOp_Xor; break;
      case T_Eqv:               result = CdlExprOp_Eqv; break;
      default:                  result = CdlExprOp_Invalid; break;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// Convert a token into an ExprOp. This way the internal token enum does
// not need to be exported in order to define the interface.
//
// In practice the higher level code will only look for a handful of
// cases, mainly EOD and the range operator, but we might as well
// do the job property.
static CdlExprOp
token_to_expr_op()
{
    CYG_REPORT_FUNCNAMETYPE("token_to_expr_op", "expr op %d");
    CdlExprOp result;

    // Many of the tokens are already handled for binary operators.
    result = token_to_binary_expr_op();
    if (CdlExprOp_Invalid == result) {
        switch(current_token) {
        case T_EOD:             result = CdlExprOp_EOD; break;
        case T_Reference:       result = CdlExprOp_Reference; break;
        case T_String:          result = CdlExprOp_StringConstant; break;
        case T_Integer:         result = CdlExprOp_IntegerConstant; break;
        case T_Double:          result = CdlExprOp_DoubleConstant; break;
        case T_Range:           result = CdlExprOp_Range; break;
        case T_Exclamation:     result = CdlExprOp_LogicalNot; break;
        case T_Tilde:           result = CdlExprOp_BitNot; break;
        case T_Questionmark:
        case T_Colon:           result = CdlExprOp_Cond; break; // best guess
        case T_Function:        result = CdlExprOp_Function; break;
        case T_OpenBracket:
        case T_CloseBracket:
        case T_Invalid:
        default:                result = CdlExprOp_Invalid; break;
        }
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

// A utility routine to turn the current token back into a string
// This is used for diagnostics.
static std::string
token_to_string()
{
    CYG_REPORT_FUNCNAME("token_to_string");
    std::string result = "";

    switch(current_token) {
      case T_EOD:               result = "<end of data>"; break;
      case T_Reference:         result = "reference to " + current_reference; break;
      case T_String:            result = "string \"" + current_string + "\""; break;
      case T_Integer:
      {
          std::string tmp;
          Cdl::integer_to_string(current_int, tmp, current_format);
          result = "integer constant " + tmp;
          break;
      }
      case T_Double:
      {
          std::string tmp;
          Cdl::double_to_string(current_double, tmp, current_format);
          result = "double constant " + tmp;
          break;
      }
      case T_Range:             result = "range operator \"to\""; break;
      case T_OpenBracket:       result = "open bracket ("; break;
      case T_CloseBracket:      result = "close bracket )"; break;
      case T_Minus:             result = "minus sign -"; break;
      case T_Plus:              result = "plus sign +"; break;
      case T_Times:             result = "multiply operator *"; break;
      case T_Divide:            result = "divide operator /"; break;
      case T_Exclamation:       result = "not operator !"; break;
      case T_Tilde:             result = "bitwise not operator ~"; break;
      case T_Questionmark:      result = "question mark ?"; break;
      case T_Remainder:         result = "remainder operator %"; break;
      case T_LeftShift:         result = "left shift operator <<"; break;
      case T_RightShift:        result = "right shift operator >>"; break;
      case T_LessThan:          result = "less-than operator <"; break;
      case T_LessEqual:         result = "less-or-equal operator <="; break;
      case T_GreaterThan:       result = "greater-than operator >"; break;
      case T_GreaterEqual:      result = "greater-or-equal operator >="; break;
      case T_Equal:             result = "equality operator =="; break;
      case T_NotEqual:          result = "not-equal operator !="; break;
      case T_BitAnd:            result = "bitwise and operator &"; break;
      case T_BitXor:            result = "bitwise xor operator ^"; break;
      case T_BitOr:             result = "bitwise or operator |"; break;
      case T_And:               result = "and operator &&"; break;
      case T_Or:                result = "or operator ||"; break;
      case T_Colon:             result = "colon"; break;
      case T_StringConcat:      result = "string concatenation operator ."; break;
      case T_Implies:           result = "implies operator"; break;
      case T_Xor:               result = "logical xor operator"; break;
      case T_Eqv:               result = "logical equivalence operator eqv"; break;
      case T_Function:          result = std::string("function call ") + CdlFunction::get_name(current_function_id); break;
      case T_Invalid:
      default:                  result = "<invalid token>"; break;
    }

    CYG_REPORT_RETURN();
    return result;
}

//}}}
//{{{  Literals                         

// ----------------------------------------------------------------------------
//{{{  process_string()                 

// The start of a string has been detected. Work out the entire string,
// allowing for backslash escapes.
static void
process_string()
{
    CYG_REPORT_FUNCNAME("process_string");
    CYG_ASSERTC('"' == current_char);
    CYG_ASSERTC("" == current_string);

    std::string result = "";

    // Move past the leading quote mark.
    next_char();
    while ('"' != current_char) {
        if (EOF == current_char) {
            throw CdlParseException("Premature end of data in string constant.\n" + get_error_location());
        } else if ('\\' == current_char) {
            // Allow \a, \b, \f, \n, \r, \t, \v, \ddd and \xhh.
            // Also copy with \newline space.
            // Any other character gets passed through unchanged.
            next_char();
            switch(current_char) {
              case EOF:
                throw CdlParseException("Premature end of data after backslash in string constant.\n" + get_error_location());
              case 'a':
                result += '\a';
                break;
              case 'b':
                result += '\b';
                break;
              case 'f':
                result += '\f';
                break;
              case 'n':
                result += '\n';
                break;
              case 'r':
                result += '\r';
                break;
              case 't':
                result += '\t';
                break;
              case 'v':
                result += '\v';
                break;
              case 'x':
              {
                cdl_int tmp = 0;
                next_char();
                if (!isxdigit(current_char)) {
                    throw CdlParseException("Non-hexadecimal digit detected in string \\x escape sequence.\n" +
                        get_error_location());
                }
                // NOTE: there is no overflow detection here.
                do {
                    tmp *= 16;
                    if (('0' <= current_char) && (current_char <= '9')) {
                        tmp += (current_char - '0');
                    } else if (('a' <= current_char) && (current_char <= 'f')) {
                        tmp += 10 + (current_char - 'a');
                    } else if (('A' <= current_char) && (current_char <= 'F')) {
                        tmp += 10 + (current_char - 'A');
                    } else {
                        CYG_FAIL("C library error, isxdigit() succeeded on non-hexadecimal character");
                    }
                    next_char();
                } while(isxdigit(current_char));
                backup_char();
                result += (char) tmp;
              }

              case '\n':
                next_char();
                while ((EOF != current_char) && isspace(current_char)) {
                    next_char();
                }
                // We have gone one too far, back up.
                backup_char();
                result += " ";
                break;

              default:
                if (('0' <= current_char) && (current_char <= '7')) {
                    // A sequence of octal digits.
                    cdl_int tmp = 0;
                    do {
                        tmp = (8 * tmp) + (current_char - '0');
                        next_char();
                    } while (('0' <= current_char) && (current_char <= '7'));
                    backup_char();
                    result += (char) tmp;
                } else {
                    // For all other backslash sequences, just add the second character
                    result += (char) current_char;
                }
            }
        } else {
            result += (char) current_char;
        }
        next_char();
    }
    // The closing quote has been reached, move past it.
    next_char();

    // And all done.
    current_token  = T_String;
    current_string = result;

    CYG_REPORT_RETURN();
}

//}}}
//{{{  process_number()                 

// The start of a number has been detected. This number may be an
// integer or a double. It is necessary to figure out where the number
// ends and invoke the appropriate Cdl:: conversion utility.
//
// Care has to be taken with termination. Consider a token such as
// 134_5. This is not a string because there are no quote marks, nor
// is it a valid reference, and because it begins with a digit it
// should be interpreted as a number. The 134 bit works fine, then
// number processing stops leaving current_char as '_'. If we are
// parsing a list expression then the following _5 will actually
// be interpreted as a reference. To avoid this, here is a utility
// which checks number completion and throws an exception if
// necessary.
static void check_number_termination()
{
    CYG_REPORT_FUNCNAME("check_number_termination");

    // End-of-data or any whitespace is ok.
    if ((EOF != current_char) && !isspace(current_char)) {
        // Any valid operator is ok as well, or brackets for that matter.
        if (('-' != current_char) && ('+' != current_char) && ('*' != current_char) &&
            ('/' != current_char) && ('!' != current_char) && ('~' != current_char) &&
            ('?' != current_char) && ('%' != current_char) && ('<' != current_char) &&
            ('>' != current_char) && ('=' != current_char) && ('&' != current_char) &&
            ('^' != current_char) && ('|' != current_char) && (':' != current_char) &&
            ('(' != current_char) && (')' != current_char)) {

            std::string tmp;
            Cdl::integer_to_string(current_int, tmp);
            throw CdlParseException("Invalid character detected after number " + tmp + "\n" + get_error_location());
        }
    }
    
    CYG_REPORT_RETURN();
}

static void
process_number()
{
    CYG_REPORT_FUNCNAME("process_number");

    std::string tmp      = "";
    bool        is_float = false;

    // Detect the special cases of 0x and octal numbers.
    if ('0' == current_char) {
        next_char();
        if (('x' == current_char) || ('X' == current_char)) {
            
            next_char();
            if (!isxdigit(current_char)) {
                throw CdlParseException("Invalid hexadecimal number, expected at least one hexadecimal digit after 0x.\n"
                                        + get_error_location());
            }
            current_int = 0;
            do {
                current_int *= 16;
                if (('0' <= current_char) && (current_char <= '9')) {
                    current_int += (current_char - '0');
                } else if (('a' <= current_char) && (current_char <= 'f')) {
                    current_int += 10 + (current_char - 'a');
                } else {
                    current_int += 10 + (current_char - 'A');
                }
                next_char();
            } while(isxdigit(current_char));
            current_token  = T_Integer;
            current_format = CdlValueFormat_Hex;
            check_number_termination();
            CYG_REPORT_RETURN();
            return;
                
        } else if (('0' <= current_char) && (current_char <= '7')) {

            current_int = 0;
            do {
                current_int *= 8;
                current_int += (current_char - '0');
                next_char();
            } while (('0' <= current_char) && (current_char <= '7'));
            current_token  = T_Integer;
            current_format = CdlValueFormat_Octal;
            check_number_termination();
            CYG_REPORT_RETURN();
            return;
            
        } else if (('8' == current_char) || ('9' == current_char)) {
            throw CdlParseException("08... and 09... are not valid  octal numbers.\n" + get_error_location());
        } else {
            // This could be plain 0, or 0.123
            // Backup, and let the rest of the code take care of things
            backup_char();
        }
    }
    
    do {
        tmp += (char) current_char;
        next_char();
    } while(isdigit(current_char));

    // If we have found a . then we have a floating point number with a fraction.
    if ('.' == current_char) {
        tmp += '.';
        next_char();
        if (!isdigit(current_char)) {
            throw CdlParseException("Invalid floating point constant, expected a digit for the fractional part.\n" +
                                    get_error_location());
        }
        is_float = true;
        do {
            tmp += (char) current_char;
            next_char();
        } while(isdigit(current_char));
    }

    // If we have found e or E then we have a floating point number with an exponent
    if (('e' == current_char) || ('E' == current_char)) {
        tmp += 'E';
        next_char();
        if (('+' == current_char) || ('-' == current_char)) {
            tmp += current_char;
            next_char();
        }
        if (!isdigit(current_char)) {
            throw CdlParseException("Invalid floating point constant, expected a digit for the exponent.\n" +
                                    get_error_location());
        }
        is_float = true;
        do {
            tmp += (char) current_char;
            next_char();
        } while(isdigit(current_char));
    }

    if (is_float) {
        if (!Cdl::string_to_double(tmp, current_double)) {
            throw CdlParseException("Invalid floating point constant `" + tmp + "'.\n" + get_error_location());
        } else {
            current_token = T_Double;
        }
    } else {
        if (!Cdl::string_to_integer(tmp, current_int)) {
            throw CdlParseException("Invalid integer constant `" + tmp + "'.\n" + get_error_location());
        } else {
            current_token = T_Integer;
        }
    }
    
    check_number_termination();
    CYG_REPORT_RETURN();
}

//}}}
//{{{  process_alphanumeric()           

// The start of an alphanumeric sequence has been detected. This may
// be a reference, a function call, or an operator like eq or to. All
// such sequences must be a valid C preprocessor name, so the only
// characters allowed are underscore, upper and lower case characters,
// and digits. The first character cannot be a digit, but that has
// been checked already.
//
// Some care has to be taken with locale's, the C library may decide
// that a character is a letter even though the same character is not
// valid as far as the preprocessor is concerned.
static void
process_alphanumeric()
{
    CYG_REPORT_FUNCNAME("process_alphanumeric");

    do {
       current_reference += (char) current_char;
       next_char();
    } while (('_' == current_char) || isdigit(current_char) ||
             (('a' <= current_char) && (current_char <= 'z')) ||
             (('A' <= current_char) && (current_char <= 'Z')));

    CYG_REPORT_RETURN();
}

//}}}
//{{{  process_special()                

// Usually an alphanumeric sequence of characters is a reference, e.g.
// CYGPKG_KERNEL. However there are only so many special characters
// available so some operators are implemented as a sequence, e.g. 
// "to". CDL also supports functions like is_substr().
//
// The data will have been collected into the current_reference string
// by a call to process_alphanumeric().

static bool
process_special()
{
    CYG_REPORT_FUNCNAMETYPE("process_special", "special %d");
    bool result = false;
    
    if ("to" == current_reference) {
        current_token  = T_Range;
        result = true;
    } else if ("implies" == current_reference) {
        current_token  = T_Implies;
        result = true;
    } else if ("xor" == current_reference) {
        current_token  = T_Xor;
        result = true;
    } else if ("eqv" == current_reference) {
        current_token  = T_Eqv;
        result = true;
    } else if (CdlFunction::is_function(current_reference.c_str(), current_function_id)) {
        current_token  = T_Function;
        result = true;
    }

    if (result) {
        current_special     = current_reference;
        current_reference   = "";
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}

//}}}
//{{{  next_token()                     

// ----------------------------------------------------------------------------
// Work out what the next token is. This includes the handling of
// strings, integers, doubles, and references.
static void
next_token()
{
    CYG_REPORT_FUNCNAMETYPE("next_token", "token %d");

    // Make sure there is no dross left lying around from the previous call.
    current_token       = T_Invalid;
    current_string      = "";
    current_reference   = "";
    current_special     = "";
    current_int         = 0;
    current_double      = 0.0;
    current_format      = CdlValueFormat_Default;
    current_function_id = 0;

    // Skip leading white space. This includes newlines, tabs, etc,
    // consider the case of:
    //    ...
    //    legal_values {
    //        1
    //        2
    //        4
    //        ..
    //    }
    //    ...
    // which is perfectly legitimate. White space inside strings
    // is handled by the string literal code, and does not get filtered
    // out here.
    //
    // Exactly which characters are white-space is implementation-defined,
    // so a special check for EOF is in order.
    while ((EOF != current_char) && isspace(current_char)) {
        next_char();
    }

    // Remember the token starting point. next_char() has actually moved
    // the index on by one.
    token_start = current_index - 1;

    // The simple cases can be handled inline, the more complicated cases
    // involve other functions
    switch(current_char) {

      case EOF:
          current_token = T_EOD;
          break;

      case '"':
          process_string();
          break;

      case '(':
          current_token = T_OpenBracket;
          next_char();
          break;

      case ')':
          current_token = T_CloseBracket;
          next_char();
          break;

          // At this level it is not possible to distinguish between
          // unary and binary operators, so no attempt is made to
          // turn - and + into part of a number.
      case '-':
          current_token = T_Minus;
          next_char();
          break;

      case '+':
          current_token = T_Plus;
          next_char();
          break;

      case '*':
          current_token = T_Times;
          next_char();
          break;

      case '/':
          current_token = T_Divide;
          next_char();
          break;

      case '!':
          next_char();
          if ('=' == current_char) {
              current_token = T_NotEqual;
              next_char();
          } else {
              current_token = T_Exclamation;
          }
          break;

      case '~':
          current_token = T_Tilde;
          next_char();
          break;

      case '?':
          current_token = T_Questionmark;
          next_char();
          break;

      case '%':
          current_token = T_Remainder;
          next_char();
          break;

      case '<':
          next_char();
          if ('<' == current_char) {
              current_token = T_LeftShift;
              next_char();
          } else if ('=' == current_char) {
              current_token = T_LessEqual;
              next_char();
          } else {
              current_token = T_LessThan;
          }
          break;

      case '>':
          next_char();
          if ('>' == current_char) {
              current_token = T_RightShift;
              next_char();
          } else if ('=' == current_char) {
              current_token = T_GreaterEqual;
              next_char();
          } else {
              current_token = T_GreaterThan;
          }
          break;

      case '=':
          next_char();
          if ('=' != current_char) {
              throw CdlParseException(std::string("Incomplete == operator in expression.\n") + get_error_location());
          } else {
              current_token = T_Equal;
              next_char();
          }
          break;

      case '&':
          next_char();
          if ('&' == current_char) {
              current_token = T_And;
              next_char();
          } else {
              current_token = T_BitAnd;
          }
          break;

      case '^':
          current_token = T_BitXor;
          next_char();
          break;

      case '|':
          next_char();
          if ('|' == current_char) {
              current_token = T_Or;
              next_char();
          } else {
              current_token = T_BitOr;
          }
          break;

      case ':':
          current_token = T_Colon;
          next_char();
          break;

      case '.':
          current_token = T_StringConcat;
          next_char();
          break;

      case ',':
          current_token = T_Comma;
          next_char();
          break;
        
      default:
          // String constants have been handled already. The only
          // valid tokens that are left are numbers, references,
          // "specials" such as the range and string equality
          // operators, and functions.
          //
          // Numbers should begin with a digit (plus and minus are
          // tokenized separately).
          //
          // References must be valid C preprocessor symbols, i.e.
          // they must begin with either a letter or an underscore.
          // The range operator is handled most conveniently as
          // a special case of a reference.
          if (isdigit(current_char)) {
              process_number();
          } else if (('_' == current_char) ||
                     (('a' <= current_char) && (current_char <= 'z')) ||
                     (('A' <= current_char) && (current_char <= 'Z'))) {
              process_alphanumeric();
              if (!process_special()) {
                  current_token = T_Reference;
              }
          } else {
              std::string msg = "Unexpected character '";
              msg += (char) current_char;
              msg += "' in expression.\n";
              msg += get_error_location();
              throw CdlParseException(msg);
          }
          break;
    }

    CYG_REPORT_RETVAL(current_token);
}

//}}}
//{{{  initialise_tokenisation()        

// ----------------------------------------------------------------------------
// This is called at the start of expression parsing. It
// sets up the appropriate statics, and provides initial
// values for current_char and current_token.
static void
initialise_tokenisation(std::string data, int index)
{
    CYG_REPORT_FUNCNAME("initialise_tokenization");

    current_data        = data;
    current_index       = static_cast<unsigned int>(index);
    token_start         = current_index;
    next_char();
    next_token();

    CYG_REPORT_RETURN();
}

//}}}

//}}}
//{{{  Syntactic analysis               

// ----------------------------------------------------------------------------
// Syntactic analysis.
//
// The BNF of CDL expressions is something like this:
//
//   <expression>   ::= <conditional>
//   <conditional>  ::= <implies> ? <conditional> : <conditional> | <implies>
//   <implies>      ::= <eqv>    [<implies op>  <implies>]      implies
//   <eqv>          ::= <or>     [<eqv op>      <eqv>]          xor, eqv        
//   <or>           ::= <and>    [<or op>       <or>]           ||
//   <and>          ::= <bitor>  [<and op>      <and>]          &&
//   <bitor>        ::= <bitxor> [<bitor op>    <bitor>]        |
//   <bitxor>       ::= <bitand> [<bitxor op>   <bitxor>]       ^
//   <bitand>       ::= <eq>     [<bitand op>   <and>]          &
//   <eq>           ::= <comp>   [<eq op>       <eq>]           == !=
//   <comp>         ::= <shift>  [<comp op>     <comp>]         < <= > >=
//   <shift>        ::= <add>    [<shift op>    <shift>]        << >>
//   <add>          ::= <mult>   [<add op>      <add>]          + - .
//   <mult>         ::= <unary>  [<mult op>     <mult>]         * / %
//   <unary>        ::= -<unary> | +<unary> | !<unary> | *<unary> | ?<unary> |
//                      ~<unary> |
//                      <string constant> | <integer constant> |
//                      <double constant> | <reference> |
//                      ( <expression> ) | <function>
//
// There are separate functions for each of these terms.

// A forward declaration, needed for bracketed subexpressions.
static void parse_expression(CdlExpression);

// A utility to add a reference to the current expression, returning
// the index.
static int
push_reference(CdlExpression expr, const std::string& reference)
{
    CYG_REPORT_FUNCNAMETYPE("push_reference", "new index %d");
    CYG_PRECONDITION_CLASSC(expr);

    CdlReference ref(reference);
    expr->references.push_back(ref);
    int result = (int) expr->references.size() - 1;

    CYG_REPORT_RETVAL(result);
    return result;
}

// A utility to add a subexpression, returning its index.
static void
push_subexpression(CdlExpression expr, const CdlSubexpression& subexpr)
{
    CYG_REPORT_FUNCNAME("push_subexpression");
    CYG_PRECONDITION_CLASSC(expr);

    expr->sub_expressions.push_back(subexpr);
    expr->first_subexpression = ((int) expr->sub_expressions.size()) - 1;

    CYG_REPORT_RETURN();
}

// Another utility to hold of the most recent subexpression
static CdlSubexpression&
current_subexpression(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("current_subexpression");

    CdlSubexpression& result = expr->sub_expressions[expr->first_subexpression];

    CYG_REPORT_RETURN();
    return result;
}

static void
parse_function(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_function");
    CYG_REPORT_FUNCARG1XV(expr);
    CYG_PRECONDITION_CLASSC(expr);

    CdlSubexpression subexpr;
    subexpr.op          = CdlExprOp_Function;
    subexpr.func        = current_function_id;

    int number_of_args  = CdlFunction::get_args_count(current_function_id);
    CYG_ASSERTC((0 < number_of_args) && (number_of_args <= CdlFunction_MaxArgs));
    std::string name    = current_special;

    // check for the opening bracket: xyzzy(arg1, arg2)
    next_token();
    if (T_OpenBracket != current_token) {
        throw CdlParseException(std::string("Expected opening bracket after function ") + name + "\n" + get_error_location());
    }
    next_token();

    int i;
    for (i = 0; i < number_of_args; i++) {
        parse_expression(expr);
        subexpr.args[i] = expr->first_subexpression;
        if (i < (number_of_args - 1)) {
            if (T_Comma != current_token) {
                throw CdlParseException(std::string("Expected comma between arguments in function ") +
                                        name + "\n" + get_error_location());
            }
            next_token();
        }
    }
    if (T_Comma == current_token) {
        throw CdlParseException(std::string("Too many arguments passed to function ") + name + "\n" + get_error_location());
    }
    if (T_CloseBracket != current_token) {
        throw CdlParseException(std::string("Expected closing bracket after function ") + name + "\n" + get_error_location());
    }
    next_token();
    
    // Allow the function implementation to check its arguments if it is so inclined.
    CdlFunction::check(expr, subexpr);
    
    push_subexpression(expr, subexpr);
    CYG_REPORT_RETURN();
}

static void
parse_unary(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_operand");
    CYG_REPORT_FUNCARG1XV(expr);
    CYG_PRECONDITION_CLASSC(expr);

    CdlSubexpression subexpr;

    switch(current_token) {
      case T_EOD :
      {
        // This warrants a special case
        throw CdlParseException("End of expression reached when expecting an operand.\n" + get_error_location());
      }

      case T_Function :
      {
          parse_function(expr);
          break;
      }
      
      case T_Reference :
      {
        subexpr.op              = CdlExprOp_Reference;
        subexpr.reference_index = push_reference(expr, current_reference);
        push_subexpression(expr, subexpr);
        next_token();
        break;
      }
      
      case T_String :
      {
        subexpr.op              = CdlExprOp_StringConstant;
        subexpr.constants       = current_string;
        push_subexpression(expr, subexpr);
        next_token();
        break;
      }
      
      case T_Integer :
      {
        subexpr.op               = CdlExprOp_IntegerConstant;
        subexpr.constants.set_integer_value(current_int, current_format);
        push_subexpression(expr, subexpr);
        next_token();
        break;
      }
      
      case T_Double :
      {
        subexpr.op              = CdlExprOp_DoubleConstant;
        subexpr.constants.set_double_value(current_double, current_format);
        push_subexpression(expr, subexpr);
        next_token();
        break;
      }
      
      case T_OpenBracket :
      {
        next_token();
        parse_expression(expr);
        if (T_CloseBracket != current_token) {
            throw CdlParseException("Missing close bracket after subexpression.\n" + get_error_location());
        }
        next_token();
        break;
      }
      
      case T_Minus :
      {
        next_token();
        parse_unary(expr);
        CdlSubexpression& last_sub      = current_subexpression(expr);
        if (CdlExprOp_IntegerConstant == last_sub.op) {
            // Do the negating inline, no need for another subexpression.
            last_sub.constants = last_sub.constants.get_integer_value() * -1;
        } else if (CdlExprOp_DoubleConstant == last_sub.op) {
            last_sub.constants = last_sub.constants.get_double_value() * -1;
        } else {
            // We could detect certain cases such as string constants etc.
            // For now don't bother.
            subexpr.op          = CdlExprOp_Negate;
            subexpr.lhs_index   = expr->first_subexpression;
            push_subexpression(expr, subexpr);
        }
        break;
      }
      
      case T_Plus :
      {
        next_token();
        parse_unary(expr);
        CdlSubexpression& last_sub      = current_subexpression(expr);
        if ((CdlExprOp_IntegerConstant == last_sub.op) || (CdlExprOp_DoubleConstant == last_sub.op)) {
            // No need to do anything here.
        } else {
            subexpr.op          = CdlExprOp_Plus;
            subexpr.lhs_index   = expr->first_subexpression;
            push_subexpression(expr, subexpr);
        }
        break;
      }

      case T_Times :
      {
          next_token();
          parse_unary(expr);
          subexpr.op            = CdlExprOp_Indirect;
          subexpr.lhs_index     = expr->first_subexpression;
          push_subexpression(expr, subexpr);
          break;
      }
      
      case T_Exclamation :
      {
          next_token();
          parse_unary(expr);
          subexpr.op            = CdlExprOp_LogicalNot;
          subexpr.lhs_index     = expr->first_subexpression;
          push_subexpression(expr, subexpr);
          break;
      }

      case T_Tilde :
      {
          next_token();
          parse_unary(expr);
          subexpr.op            = CdlExprOp_BitNot;
          subexpr.lhs_index     = expr->first_subexpression;
          push_subexpression(expr, subexpr);
          break;
      }

      case T_Questionmark:
      {
          // This is the `active' operator, it can only be applied directly to a reference.
          next_token();
          parse_unary(expr);
          CdlSubexpression& last_sub = current_subexpression(expr);
          if (CdlExprOp_Reference != last_sub.op) {
              throw CdlParseException("The active operator ? can only be applied directly to a reference.\n" +
                                      get_error_location());
          }
          // There is no point in creating a new subexpression object, just modify
          // the existing one. This has the useful side effect of avoiding
          // reference substitution in the eval code.
          last_sub.op           = CdlExprOp_Active;
          break;
      }
      default:
      {
        throw CdlParseException("Unexpected token `" + token_to_string() + "', expecting an operand.\n" +
                                get_error_location());
      }
    }

    CYG_REPORT_RETURN();
}

static void
parse_multiply(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_multiply");
    
    parse_unary(expr);
    while ((T_Times == current_token) || (T_Divide == current_token) || (T_Remainder == current_token)) {

        CdlSubexpression subexpr;
        subexpr.op      =
            (T_Times  == current_token) ? CdlExprOp_Multiply :
            (T_Divide == current_token) ? CdlExprOp_Divide : CdlExprOp_Remainder;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_unary(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_add(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_add");
    
    parse_multiply(expr);
    while ((T_Plus == current_token)  ||
           (T_Minus == current_token) ||
           (T_StringConcat == current_token)) {

        CdlSubexpression subexpr;
        subexpr.op = (T_Plus == current_token) ? CdlExprOp_Add :
                     (T_Minus == current_token) ? CdlExprOp_Subtract :
                     CdlExprOp_StringConcat;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_multiply(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_shift(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_shift");
    
    parse_add(expr);
    while ((T_LeftShift == current_token) || (T_RightShift == current_token)) {

        CdlSubexpression subexpr;
        subexpr.op = (T_LeftShift == current_token) ? CdlExprOp_LeftShift : CdlExprOp_RightShift;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_add(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_comparison(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_comparison");
    
    parse_shift(expr);
    while ((T_LessThan == current_token)    || (T_LessEqual    == current_token) ||
           (T_GreaterThan == current_token) || (T_GreaterEqual == current_token))  {

        CdlSubexpression subexpr;
        subexpr.op =
            (T_LessThan    == current_token) ? CdlExprOp_LessThan : 
            (T_LessEqual   == current_token) ? CdlExprOp_LessEqual :
            (T_GreaterThan == current_token) ? CdlExprOp_GreaterThan : CdlExprOp_GreaterEqual;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_shift(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_equals(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_equals");
    
    parse_comparison(expr);
    while ((T_Equal == current_token) ||
           (T_NotEqual == current_token)) {

        CdlSubexpression subexpr;
        subexpr.op = (T_Equal == current_token) ? CdlExprOp_Equal : CdlExprOp_NotEqual;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_comparison(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_bitand(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_bitand");
    
    parse_equals(expr);
    while (T_BitAnd == current_token) {

        CdlSubexpression subexpr;
        subexpr.op = CdlExprOp_BitAnd;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_equals(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_bitxor(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_bitxor");
    
    parse_bitand(expr);
    while (T_BitXor == current_token) {

        CdlSubexpression subexpr;
        subexpr.op = CdlExprOp_BitXor;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_bitand(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_bitor(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_bitor");
    
    parse_bitxor(expr);
    while (T_BitOr == current_token) {

        CdlSubexpression subexpr;
        subexpr.op = CdlExprOp_BitOr;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_bitxor(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_and(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_and");
    parse_bitor(expr);
    while (T_And == current_token) {

        CdlSubexpression subexpr;
        subexpr.op = CdlExprOp_And;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_bitor(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_or(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_or");

    parse_and(expr);
    while (T_Or == current_token) {

        CdlSubexpression subexpr;
        subexpr.op = CdlExprOp_Or;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_and(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_eqv(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_eqv");

    parse_or(expr);
    while ((T_Xor == current_token) || (T_Eqv == current_token)) {
        
        CdlSubexpression subexpr;
        subexpr.op = (T_Xor == current_token) ? CdlExprOp_Xor : CdlExprOp_Eqv;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_or(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_implies(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_implies");

    parse_eqv(expr);
    while (T_Implies == current_token) {
        
        CdlSubexpression subexpr;
        subexpr.op = CdlExprOp_Implies;
        subexpr.lhs_index = expr->first_subexpression;
        
        next_token();
        parse_eqv(expr);

        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_conditional(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_conditional");

    parse_implies(expr);
    if (T_Questionmark == current_token) {
        CdlSubexpression subexpr;
        subexpr.op = CdlExprOp_Cond;
        subexpr.lhs_index = expr->first_subexpression;

        next_token();
        parse_conditional(expr);
        subexpr.rhs_index = expr->first_subexpression;

        if (T_Colon != current_token) {
            throw CdlParseException("Expected colon in conditional expression.\n" + get_error_location());
        }

        next_token();
        parse_conditional(expr);
        subexpr.rrhs_index = expr->first_subexpression;

        push_subexpression(expr, subexpr);
    }
    
    CYG_REPORT_RETURN();
}

static void
parse_expression(CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("parse_expression");

    parse_conditional(expr);
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// The entry point.
void
CdlExpressionBody::continue_parse(CdlExpression expr, std::string data, int& index, CdlExprOp& token, int& token_end)
{
    CYG_REPORT_FUNCNAME("CdlExpression::continue_parse");
    CYG_REPORT_FUNCARG1XV(expr);
    CYG_PRECONDITION_CLASSC(expr);
    CYG_PRECONDITIONC((CdlExprOp_Invalid == token) || (CdlExprOp_And == token));

    int current_subexpr = expr->first_subexpression;
    initialise_tokenisation(data, index);
    parse_expression(expr);
    if (CdlExprOp_And == token) {
        CdlSubexpression subexpr;
        subexpr.op        = CdlExprOp_And;
        subexpr.lhs_index = current_subexpr;
        subexpr.rhs_index = expr->first_subexpression;
        push_subexpression(expr, subexpr);
    }
    token       = token_to_expr_op();
    index       = token_start;
    token_end   = current_index;

    CYG_REPORT_RETURN();
}

//}}}

//}}}
//{{{  Expression Evaluation            

// ----------------------------------------------------------------------------
// Expression evaluation. This always happens in the context of a
// particular toplevel. The parsed expression is held in what amounts
// to a simple tree, so evaluation involves some recursion and a big
// switch statement.

static void
evaluate_subexpr(CdlEvalContext& context, CdlExpression expr, int subexpr_index, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("evaluate_subexpr");
    CYG_REPORT_FUNCARG2XV(expr, subexpr_index);
    CYG_ASSERTC((subexpr_index >= 0) && ((unsigned int)subexpr_index < expr->sub_expressions.size()));

    const CdlSubexpression& subexpr = expr->sub_expressions[subexpr_index];
    switch(subexpr.op) {
    case CdlExprOp_StringConstant :
    case CdlExprOp_IntegerConstant :
    case CdlExprOp_DoubleConstant :
    {
        result = subexpr.constants;
        break;
    }
    case CdlExprOp_Function :
    {
        CdlFunction::eval(context, expr, subexpr, result);
        break;
    }
    case CdlExprOp_Reference :
    {
        // This expression may be happening in the context of a particular
        // property. If so then the destination may or may not be resolved,
        // and this is significant in the context of loading and unloading.
        // Alternatively this expression may be being evaluated inside
        // some Tcl code, with no particular context.
        CdlNode destination = 0;
        if (0 != context.property) {
            // There is a property, use the bound/unbound reference.
            destination = expr->references[subexpr.reference_index].get_destination();
        } else {
            // The destination name can be retrieved, but we still need some
            // way of resolving it.
            if (0 != context.toplevel) {
                std::string destination_name = expr->references[subexpr.reference_index].get_destination_name();
                destination = context.toplevel->lookup(destination_name);
            }
        }
        if (0 == destination) {
            // There are two ways of handling this.
            //   1) throw an eval exception, which will usually result
            //      in a new conflict object
            //   2) substitute a value of 0.
            // There should already be a conflict object for an
            // unresolved reference, and having two conflicts for
            // essentially the same error is not useful. Using a value
            // of 0 allows things to continue for a bit longer. It is
            // consistent with active vs. inactive values, gives
            // basically the right result for "requires" properties,
            // and so on.
            //
            // For now option (2) has it, but this decision may be
            // reversed in future.
            result = false;
        } else {
            CdlValuable valuable = dynamic_cast<CdlValuable>(destination);
            if (0 == valuable) {
                // This is a serious problem, an exception is warranted.
                throw CdlEvalException("The expression references `" + destination->get_class_name() + " " +
                                       destination->get_name() + "' which does not have a value.");
            } else {
                CdlSimpleValue::eval_valuable(context, valuable, result);
            }
        }
        break;
    }
    case CdlExprOp_Negate :
    {
        // Unary -. Evaluate the target. If it is numeric, fine. Otherwise
        // an error is warranted.
        evaluate_subexpr(context, expr, subexpr.lhs_index, result);
        if (result.has_integer_value()) {
            result.set_integer_value(-1 * result.get_integer_value());
        } else if (result.has_double_value()) {
            result.set_double_value(-1.0 * result.get_double_value());
        } else {
            throw CdlEvalException("Attempt to negate non-numeric value `" + result.get_value() + "'.");
        }
        break;
    }
    case CdlExprOp_Plus :
    {
        // Unary +. Essentially this just checks that the current value is numeric.
        evaluate_subexpr(context, expr, subexpr.lhs_index, result);
        if ((!result.has_integer_value()) && (!result.has_double_value())) {
            throw CdlEvalException("Attempt to apply unary + operator to non-numeric value `" + result.get_value() + "'.");
        }
        break;
    }
    case CdlExprOp_LogicalNot :
    {
        // !x
        evaluate_subexpr(context, expr, subexpr.lhs_index, result);
        if (result.get_bool_value()) {
            result = false;;
        } else {
            result = true;
        }
        result.set_value_format(CdlValueFormat_Default);
        break;
    }
    case CdlExprOp_BitNot :
    {
        // ~x. The operand must be an integer value.
        evaluate_subexpr(context, expr, subexpr.lhs_index, result);
        if (result.has_integer_value()) {
            cdl_int tmp = result.get_integer_value();
            result = ~tmp;
        } else {
            throw CdlEvalException("Attempt to apply unary ~ operator to non-integer value `" + result.get_value() + "'.");
        }
        break;
    }
    case CdlExprOp_Indirect :
    {
        // *x. The operand must evaluate to a string, and that string should be
        // the name of a CdlValuable object.
        CdlNode destination = 0;
        evaluate_subexpr(context, expr, subexpr.lhs_index, result);
        std::string name = result.get_value();
        
        if (0 != context.toplevel) {
            destination = context.toplevel->lookup(name);
        } else {
            CYG_FAIL("This situation should probably never happen.");
        }
        
        if (0 == destination) {
            throw CdlEvalException("Attempt to apply unary indirection operator * to `" + name +
                                   "', which is not the name of a known CDL entity.");
        } else {
            CdlValuable valuable = dynamic_cast<CdlValuable>(destination);
            if (0 == valuable) {
                throw CdlEvalException("Attempt to apply unary indirection operator * to `" + name +
                                       "', which does not have a value.");
            } else {
                CdlSimpleValue::eval_valuable(context, valuable, result);
            }
        }
        break;
    }
    case CdlExprOp_Active :
    {
        // ?x. If x is currently unresolved then default to 0.
        // See the CdlExprOp_Reference code above for a similar case.
        CdlNode destination = 0;
        if (0 != context.property) {
            destination =  expr->references[subexpr.reference_index].get_destination();
        } else {
            if (0 != context.toplevel) {
                std::string destination_name = expr->references[subexpr.reference_index].get_destination_name();
                destination = context.toplevel->lookup(destination_name);
            }
        }

        bool active = false;
        if ((0 != destination) && context.transaction->is_active(destination)) {
            active = true;
        }
        if (active) {
            result = true;
        } else {
            result = false;
        }
        break;
    }
    case CdlExprOp_Multiply :
    {
        // x * y. For now this only makes sense for numerical data,
        // but it is possible to mix and match integer and double
        // precision data.
        //
        // Strictly speaking the rhs need only be evaluated if it
        // is known that the lhs is numeric.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if ((!(lhs.has_integer_value() || lhs.has_double_value())) ||
            (!(rhs.has_integer_value() || rhs.has_double_value()))) {
            throw CdlEvalException("Attempt to multiply non-numerical values: `" + lhs.get_value() + "' * `" +
                                   rhs.get_value() + "'.");
        }
        if (lhs.has_integer_value() && rhs.has_integer_value()) {
            result = lhs.get_integer_value() * rhs.get_integer_value();
        } else {
            result = lhs.get_double_value() * rhs.get_double_value();
        }
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_Divide :
    {
        // x / y. Basically the same as multiplication, apart from a check for
        // division by zero.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if ((!(lhs.has_integer_value() || lhs.has_double_value())) ||
            (!(rhs.has_integer_value() || rhs.has_double_value()))) {
            throw CdlEvalException("Attempt to divide non-numerical values: `" + lhs.get_value() + "' / `" +
                                   rhs.get_value() + "'.");
        }
        if (lhs.has_integer_value() && rhs.has_integer_value()) {
            cdl_int rhs_val = rhs.get_integer_value();
            if (0 == rhs_val) {
                throw CdlEvalException("Division by zero error: `" + lhs.get_value() + "' / `" + rhs.get_value() + "'.");
            } else {
                result = lhs.get_integer_value() / rhs_val;
            }
        } else {
            double rhs_val = rhs.get_double_value();
            if (0.0 == rhs_val) {
                throw CdlEvalException("Division by zero error: `" + lhs.get_value() + "' / `" + rhs.get_value() + "'.");
            }
            result = lhs.get_double_value() / rhs_val;
        }
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_Remainder :
    {
        // x % y. Both operands must be integral.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if (!(lhs.has_integer_value() && rhs.has_integer_value())) {
            throw CdlEvalException("Attempt to use the remainder operator on non integral data: `" +
                                   lhs.get_value() + "' % `" + rhs.get_value() + "'.");
        }
        cdl_int rhs_val = rhs.get_integer_value();
        if (0 == rhs_val) {
            throw CdlEvalException("Division by zero error: `" + lhs.get_value() + "' % `" + rhs.get_value() + "'.");
        }
        result = lhs.get_integer_value() % rhs_val;
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_Add :
    {
        // x + y. For now this only makes sense for numerical data,
        // but it is possible to mix and match integer and double
        // precision data. Arguably for string data this operator
        // should mean concatenation, but it would probably be
        // safer to have a separate operator for that.
        //
        // Strictly speaking the rhs need only be evaluated if it
        // is known that the lhs is numeric.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if ((!(lhs.has_integer_value() || lhs.has_double_value())) ||
            (!(rhs.has_integer_value() || rhs.has_double_value()))) {
            throw CdlEvalException("Attempt to add non-numerical values: `" + lhs.get_value() + "' + `" +
                                   rhs.get_value() + "'.");
        }
        if (lhs.has_integer_value() && rhs.has_integer_value()) {
            result = lhs.get_integer_value() + rhs.get_integer_value();
        } else {
            result = lhs.get_double_value() + rhs.get_double_value();
        }
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_Subtract :
    {
        // x - y. Again only numerical data is supported for now.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if ((!(lhs.has_integer_value() || lhs.has_double_value())) ||
            (!(rhs.has_integer_value() || rhs.has_double_value()))) {
            throw CdlEvalException("Attempt to subtract non-numerical values: `" + lhs.get_value() + "' - `" +
                                   rhs.get_value() + "'.");
        }
        if (lhs.has_integer_value() && rhs.has_integer_value()) {
            result = lhs.get_integer_value() - rhs.get_integer_value();
        } else {
            result = lhs.get_double_value() - rhs.get_double_value();
        }
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_LeftShift :
    {
        // x << y. Both operands must be integral. For now there is no
        // check on the value of y.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if (!(lhs.has_integer_value() && rhs.has_integer_value())) {
            throw CdlEvalException("Attempt to use the left-shift operator on non integral data: `" +
                                   lhs.get_value() + "' << `" + rhs.get_value() + "'.");
        }
        result = lhs.get_integer_value() << rhs.get_integer_value();
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_RightShift :
    {
        // x >> y. Both operands must be integral. For now there is no
        // check on the value of y.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if (!(lhs.has_integer_value() && rhs.has_integer_value())) {
            throw CdlEvalException("Attempt to use the right-shift operator on non integral data: `" +
                                   lhs.get_value() + "' >> `" + rhs.get_value() + "'.");
        }
        result = lhs.get_integer_value() >> rhs.get_integer_value();
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_LessThan :
    case CdlExprOp_LessEqual :
    case CdlExprOp_GreaterThan :
    case CdlExprOp_GreaterEqual :
    {
        // x < y, and similar comparison operators. These share
        // sufficient code to warrant a common implementation. Only
        // numerical data is supported for now. These operator could
        // be interpreted as e.g. substring operations, but arguably
        // separate operators would be better for that.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if ((!(lhs.has_integer_value() || lhs.has_double_value())) ||
            (!(rhs.has_integer_value() || rhs.has_double_value()))) {
            
            std::string op_str =
                (CdlExprOp_LessThan    == subexpr.op) ? "<" :
                (CdlExprOp_LessEqual   == subexpr.op) ? "<=" :
                (CdlExprOp_GreaterThan == subexpr.op) ? ">" : ">=";

            throw CdlEvalException("Attempt to compare non-numerical values: `" + lhs.get_value() +
                                   "' " + op_str + " `" + rhs.get_value() + "'.");
        }
        bool val = false;
        if (lhs.has_integer_value() && rhs.has_integer_value()) {
            cdl_int lhs_val = lhs.get_integer_value();
            cdl_int rhs_val = rhs.get_integer_value();
            val =
                (CdlExprOp_LessThan    == subexpr.op) ? (lhs_val <  rhs_val) :
                (CdlExprOp_LessEqual   == subexpr.op) ? (lhs_val <= rhs_val) :
                (CdlExprOp_GreaterThan == subexpr.op) ? (lhs_val >  rhs_val) : (lhs_val >= rhs_val);
        } else {
            double lhs_val = lhs.get_double_value();
            double rhs_val = rhs.get_double_value();
            val =
                (CdlExprOp_LessThan    == subexpr.op) ? (lhs_val <  rhs_val) :
                (CdlExprOp_LessEqual   == subexpr.op) ? (lhs_val <= rhs_val) :
                (CdlExprOp_GreaterThan == subexpr.op) ? (lhs_val >  rhs_val) : (lhs_val >= rhs_val);
        }
        result = val;
        break;
    }
    case CdlExprOp_Equal :
    {
        // x == y. For numerical data this should be a numerical comparison.
        // Otherwise a string comparison has to be used.
        bool val = false;
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if ((lhs.has_integer_value() || lhs.has_double_value()) &&
            (rhs.has_integer_value() || rhs.has_double_value())) {

            if (lhs.has_integer_value() && rhs.has_integer_value()) {
                if (lhs.get_integer_value() == rhs.get_integer_value()) {
                    val = true;
                } else {
                    val = false;
                }
            } else {
                if (lhs.get_double_value() == rhs.get_double_value()) {
                    val = true;
                } else {
                    val = false;
                }
                  
            }
        } else {
            // At least one of the two sides is non-numerical. Do a string comparison.
            if (lhs.get_value() == rhs.get_value()) {
                val = true;
            } else {
                val = false;
            }
        }
        result = val;
        break;
    }
    case CdlExprOp_NotEqual :
    {
        // x != y. For numerical data this should be a numerical comparison.
        // Otherwise a string comparison has to be used.
        bool val = false;
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if ((lhs.has_integer_value() || lhs.has_double_value()) &&
            (rhs.has_integer_value() || rhs.has_double_value())) {

            if (lhs.has_integer_value() && rhs.has_integer_value()) {
                if (lhs.get_integer_value() != rhs.get_integer_value()) {
                    val = true;
                } else {
                    val = false;
                }
            } else {
                if (lhs.get_double_value() != rhs.get_double_value()) {
                    val = true;
                } else {
                    val = false;
                }
                  
            }
        } else {
            // At least one of the two sides is non-numerical. Do a string comparison.
            if (lhs.get_value() != rhs.get_value()) {
                val = true;
            } else {
                val = false;
            }
        }
        result = val;
        break;
    }
    case CdlExprOp_BitAnd :
    {
        // x & y. Only integer data is supported.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if (!(lhs.has_integer_value() && rhs.has_integer_value())) {
            throw CdlEvalException("Attempt to use the bitwise and operator on non integral data: `" +
                                   lhs.get_value() + "' & `" + rhs.get_value() + "'.");
        }
        result = lhs.get_integer_value() & rhs.get_integer_value();
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_BitXor :
    {
        // x ^ y. Only integer data is supported.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if (!(lhs.has_integer_value() && rhs.has_integer_value())) {
            throw CdlEvalException("Attempt to use the bitwise xor operator on non integral data: `" +
                                   lhs.get_value() + "' ^ `" + rhs.get_value() + "'.");
        }
        result = lhs.get_integer_value() ^ rhs.get_integer_value();
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_BitOr :
    {
        // x | y. Only integer data is supported.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        if (!(lhs.has_integer_value() && rhs.has_integer_value())) {
            throw CdlEvalException("Attempt to use the bitwise or operator on non integral data: `" +
                                   lhs.get_value() + "' | `" + rhs.get_value() + "'.");
        }
        result = lhs.get_integer_value() | rhs.get_integer_value();
        result.set_value_format(lhs, rhs);
        break;
    }
    case CdlExprOp_And :
    {
        // x && y. Both sides should be interpreted as boolean values,
        // and "y" should only be evaluated if necessary.
        evaluate_subexpr(context, expr, subexpr.lhs_index, result);
        if (!result.get_bool_value()) {
            result = false;
        } else {
            evaluate_subexpr(context, expr, subexpr.rhs_index, result);
            if (result.get_bool_value()) {
                result = true;
            } else {
                result = false;
            }
        }
        break;
    }
    case CdlExprOp_Or :
    {
        // x || y. Both sides should be interpreted as boolean values,
        // and "y" should only be evaluated if necessary.
        evaluate_subexpr(context, expr, subexpr.lhs_index, result);
        if (result.get_bool_value()) {
            result = true;
        } else {
            evaluate_subexpr(context, expr, subexpr.rhs_index, result);
            if (result.get_bool_value()) {
                result = true;
            } else {
                result = false;
            }
        }
        break;
    }
    case CdlExprOp_Xor :
    {
        // x xor y. Both sides should be interpreted as boolean values.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);

        bool lhs_bool = lhs.get_bool_value();
        bool rhs_bool = rhs.get_bool_value();
        if ((lhs_bool && !rhs_bool) || (!lhs_bool && rhs_bool)) {
            result = true;
        } else {
            result = false;
        }
        
        break;
    }
    case CdlExprOp_Eqv :
    {
        // x eqv y. Both sides should be interpreted as boolean values.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);

        bool lhs_bool = lhs.get_bool_value();
        bool rhs_bool = rhs.get_bool_value();
        if ((!lhs_bool && !rhs_bool) || (lhs_bool && rhs_bool)) {
            result = true;
        } else {
            result = false;
        }
        
        break;
    }
    case CdlExprOp_Implies :
    {
        // x implies y. Both sides should be interpreted as boolean values.
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);

        bool lhs_bool = lhs.get_bool_value();
        bool rhs_bool = rhs.get_bool_value();
        if (!lhs_bool || rhs_bool) {
            result = true;
        } else {
            result = false;
        }
        
        break;
    }
    case CdlExprOp_Cond :
    {
        // x ? a : b.
        // First evaluate the condition. Then evaluate either the second
        // or third argument, as appropriate.
        evaluate_subexpr(context, expr, subexpr.lhs_index, result);
        if (result.get_bool_value()) {
            evaluate_subexpr(context, expr, subexpr.rhs_index, result);
        } else {
            evaluate_subexpr(context, expr, subexpr.rrhs_index, result);
        }
        break;
    }
    case CdlExprOp_StringConcat :
    {
        // a . b
        CdlSimpleValue lhs;
        CdlSimpleValue rhs;
        evaluate_subexpr(context, expr, subexpr.lhs_index, lhs);
        evaluate_subexpr(context, expr, subexpr.rhs_index, rhs);
        result = lhs.get_value() + rhs.get_value();
        break;
    }

    default:
        break;
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
void
CdlExpressionBody::eval_internal(CdlEvalContext& context, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("CdlExpression::eval_internal)");
    CYG_REPORT_FUNCARG3XV(this, &context, &result);
    CYG_INVARIANT_THISC(CdlExpressionBody);
    CYG_PRECONDITION_CLASSOC(context);

    evaluate_subexpr(context, this, first_subexpression, result);
    
    CYG_REPORT_RETURN();
}

void
CdlExpressionBody::eval_subexpression(CdlEvalContext& context, int index, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("CdlExpression::eval_subexpression)");
    CYG_REPORT_FUNCARG4XV(this, &context, index, &result);
    CYG_INVARIANT_THISC(CdlExpressionBody);
    CYG_PRECONDITION_CLASSOC(context);

    evaluate_subexpr(context, this, index, result);
    
    CYG_REPORT_RETURN();
}

//}}}

//{{{  CdlExpression                    

//{{{  Construction                                     

// ----------------------------------------------------------------------------
// Ordinary expressions.
//
// The default constructor is private and does very little. Expressions
// are created primarily by means of the parse() member function. There
// is an argument for having constructors that take the same arguments
// as the parse() member functions and relying on exception handling,
// but that gets tricky for goal expressions and continue_parse().
//
// The copy constructor is protected and is used when creating e.g.
// a default_value property object, which inherits from the ordinary
// expression class. Again it might be better to do the parsing in
// the constructor itself.
//
// The assignment operator is private and illegal.

CdlExpressionBody::CdlExpressionBody()
{
    CYG_REPORT_FUNCNAME("CdlExpression:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    expression_string           = "";
    first_subexpression         = -1;

    cdlexpressionbody_cookie    = CdlExpressionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlExpressionBody::CdlExpressionBody(const CdlExpressionBody& original)
{
    CYG_REPORT_FUNCNAME("CdlExpression:: copy constructor");
    CYG_REPORT_FUNCARG2XV(this, &original);
    CYG_INVARIANT_CLASSOC(CdlExpressionBody, original);

    // Sub-expressions are simple structs, so this should result in a bit-wise
    // copy of each vector element
    sub_expressions     = original.sub_expressions;

    // Simple scalar
    first_subexpression = original.first_subexpression;

    // The CdlReference class has a valid copy constructor and assignment
    // operator, provided that the reference is not yet bound. This should
    // be true when this copy constructor gets invoked, after parsing
    // and during the construction of a derived property object.
    references          = original.references;
    expression_string   = original.expression_string;

    cdlexpressionbody_cookie    = CdlExpressionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

//}}}
//{{{  check_this()                                     

// ----------------------------------------------------------------------------
// check_this(). Expression objects can exist before any parsing has
// happened, not to mention in the middle of parsing. The
// first_subexpression field can be used to detect this.

bool
CdlExpressionBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlExpressionBody_Magic != cdlexpressionbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    if (-1 == first_subexpression) {
        return true;
    }

    switch(zeal) {
      case cyg_system_test    :
      case cyg_extreme        :
      case cyg_thorough       :
      {
          for (std::vector<CdlReference>::const_iterator i = references.begin(); i != references.end(); i++) {
              if (!i->check_this(cyg_quick)) {
                  return false;
              }
          }
      }
      case cyg_quick          :
          if ((unsigned)first_subexpression >= sub_expressions.size()) {
              return false;
          }
      case cyg_trivial        :
      case cyg_none           :
        break;
    }

    return true;
}

//}}}
//{{{  Destruction                                      

CdlExpressionBody::~CdlExpressionBody()
{
    CYG_REPORT_FUNCNAME("CdlExpression::destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlexpressionbody_cookie    = CdlExpressionBody_Invalid;
    first_subexpression         = -1;
    sub_expressions.clear();
    expression_string           = "";

    // This assumes that all references have been unbound already by
    // higher-level destructors.
    references.clear();

    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Parsing - exported interface                     

// ----------------------------------------------------------------------------
// parse(string) invokes parse(string, ...) and checks that the expression
// has terminated with EOD. Parsing of list expressions etc. can terminate
// with some other token.
//
// parse(string, ...) allocates the expression object and invokes
// continue_parse().
//
// continue_parse() is supposed to do all the hard work.

CdlExpression
CdlExpressionBody::parse(std::string data)
{
    CYG_REPORT_FUNCNAMETYPE("CdlExpression::parse", "result %p");

    CdlExpression       result  = 0;
    int                 index   = 0;
    CdlExprOp           next_op = CdlExprOp_Invalid;
    int                 end_index;

    result = parse(data, index, next_op, end_index);
    
    // Either there has already been a parsing or out-of-memory
    // exception, or we should be at the end of the expression string.
    if (CdlExprOp_EOD != next_op) {
        delete result;
        throw CdlParseException("Unexpected data at end of expression.\n" + get_error_location());
    }

    // Keep a copy of the original string for diagnostics purposes.
    result->expression_string = data;

    CYG_REPORT_RETVAL(result);
    return result;
}

CdlExpression
CdlExpressionBody::parse(std::string data, int& index, CdlExprOp& next_token, int& token_end)
{
    CYG_REPORT_FUNCNAMETYPE("CdlExpression::parse", "result %d");

    CdlExpression result = new CdlExpressionBody;

    try {
        continue_parse(result, data, index, next_token, token_end);
    }
    catch (...) {
        delete result;
        throw;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  update()                                         

// ----------------------------------------------------------------------------
// There has been a change in the toplevel which involves entities being
// created or destroyed, and reference resolution is required.

bool
CdlExpressionBody::update(CdlTransaction transaction, CdlNode source, CdlProperty source_prop, CdlNode dest, CdlUpdate change)
{
    CYG_REPORT_FUNCNAMETYPE("CdlExpression::update", "result %d");
    CYG_REPORT_FUNCARG6XV(this, transaction, source, source_prop, dest, change);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(source);
    CYG_PRECONDITION_CLASSC(source_prop);

    CdlToplevel toplevel        = source->get_toplevel();
    bool        result          = false;
    std::vector<CdlReference>::iterator ref_i;

    switch(change) {
      case CdlUpdate_Loaded:
      {
        // The source package has just been loaded. Try to resolve every
        // reference, creating CdlConflict objects where necessary.
        CYG_ASSERTC(0 == dest);
        for (ref_i = references.begin(); ref_i != references.end(); ref_i++) {
            dest = toplevel->lookup(ref_i->get_destination_name());
            if (0 == dest) {
                CdlConflict_UnresolvedBody::make(transaction, source, source_prop, ref_i->get_destination_name());
            } else {
                ref_i->bind(source, source_prop, dest);
            }
        }
        result = true;
        break;
      }

      case CdlUpdate_Unloading:
      {
        // The source package is being unloaded. Unbind all currently bound references.
        // Also destroy any unresolved conflicts.
        CYG_ASSERTC(0 == dest);
        for (ref_i = references.begin(); ref_i != references.end(); ref_i++) {
            dest = ref_i->get_destination();
            if (0 != dest) {
                ref_i->unbind(source, source_prop);
            }
        }
        result = true;
        break;
      }

      case CdlUpdate_Created :
      {
        
        // A previously unresolved reference can now be resolved.
        // It is necessary to search the vector for an unresolved
        // reference with the desired name, and do the binding.
        // This search may fail in the case of list expressions.
        CYG_ASSERT_CLASSC(dest);
        std::string dest_name = dest->get_name();
        for (ref_i = references.begin(); !result && (ref_i != references.end()); ref_i++) {
            if ((dest_name == ref_i->get_destination_name()) && (0 == ref_i->get_destination())) {
                ref_i->bind(source, source_prop, dest);
                result = true;
                
                std::vector<CdlConflict> conflicts;
                std::vector<CdlConflict>::iterator conf_i;
                transaction->get_structural_conflicts(source, source_prop, &CdlConflict_UnresolvedBody::test, conflicts);
                for (conf_i = conflicts.begin(); conf_i != conflicts.end(); conf_i++) {
                    CdlConflict_Unresolved real_conf = dynamic_cast<CdlConflict_Unresolved>(*conf_i);
                    CYG_ASSERTC(0 != real_conf);
                    if (dest_name == real_conf->get_target_name()) {
                        transaction->clear_conflict(real_conf);
                        break;
                    }
                }
                CYG_ASSERTC(conf_i != conflicts.end());
            }
        }
        break;
      }

      case CdlUpdate_Destroyed :
      {
        // A previously resolved reference is about to become illegal.
        // Search the vector for a resolved reference object matching
        // the destination, and unbind it. Also create a conflict
        // object. The search can fail in the case of list expressions
        CYG_ASSERT_CLASSC(dest);
        for (ref_i = references.begin(); !result && (ref_i != references.end()); ref_i++) {
            if (dest == ref_i->get_destination()) {
                ref_i->unbind(source, source_prop);
                CdlConflict_UnresolvedBody::make(transaction, source, source_prop, ref_i->get_destination_name());
                result = true;
            }
        }
        break;
      }

      default :
          CYG_FAIL("Illegal change type passed to CdlExpression::update");
          break;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Evaluation                                       

// ----------------------------------------------------------------------------
// Expression evaluation. At the end of the day everything filters through
// to eval_internal() which should all the hard work.
//
// The eval() member function handles EvalException conflicts. The
// eval_internal() member function does not, and is used for list
// and goal expressions as well.

void
CdlExpressionBody::eval(CdlEvalContext& context, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("CdlExpression::eval");

    try {
        
        eval_internal(context, result);
        
        // Evaluation has succeeded, so if there was an EvalException
        // conflict get rid of it. This can only happen in the context
        // of a transaction.
        if ((0 != context.transaction) && (0 != context.node) && (0 != context.property)) {
            context.transaction->clear_conflicts(context.node, context.property, &CdlConflict_EvalExceptionBody::test);
        }
        
    } catch(CdlEvalException e) {

        if ((0 != context.transaction) && (0 != context.node) && (0 != context.property)) {

            CdlConflict conflict = context.transaction->get_conflict(context.node, context.property,
                                                                              &CdlConflict_EvalExceptionBody::test);
            if (0 == conflict) {
                CdlConflict_EvalExceptionBody::make(context.transaction, context.node, context.property, e.get_message());
            } else {
                
                CdlConflict_EvalException eval_conf = dynamic_cast<CdlConflict_EvalException>(conflict);
                CYG_ASSERTC(0 != eval_conf);
                if (eval_conf->get_explanation() != e.get_message()) {
                    
                    // Replace the conflict object. That way higher level code gets informed
                    // there has been a change.
                    context.transaction->clear_conflict(conflict);
                    CdlConflict_EvalExceptionBody::make(context.transaction, context.node, context.property, e.get_message());
                }
            }
        }

        throw;
    }
}

//}}}
//{{{  Misc                                             

// ----------------------------------------------------------------------------

std::string
CdlExpressionBody::get_original_string() const
{
    CYG_REPORT_FUNCNAME("CdlExpression::get_original_string");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return expression_string;
}

//}}}

//}}}
//{{{  CdlListExpression                

//{{{  Construction                             

// ----------------------------------------------------------------------------
// The normal sequence of events is:
//
// 1) higher level code calls CdlListExpressionbody::parse()
// 2) this static member creates a new and empty list expression object.
//    The constructor need not do very much.
// 3) the parse() member then fills in the newly created object
// 4) the object is returned to higher-level code
// 5) usually the list expression will now become part of
//    a property object by means of a copy constructor.
//
// The only complication is that a list expression contains a vector
// of CdlExpression pointers which must be freed during the destructor.
// The copy constructor does not make duplicates of the individual
// expression objects, instead ownership is transferred.

CdlListExpressionBody::CdlListExpressionBody()
{
    CYG_REPORT_FUNCNAME("CdlListExpression:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    expression_string           = "";

    cdllistexpressionbody_cookie = CdlListExpressionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlListExpressionBody::CdlListExpressionBody(const CdlListExpressionBody& original)
{
    CYG_REPORT_FUNCNAME("CdlListExpression:: copy constructor");
    CYG_REPORT_FUNCARG2XV(this, &original);
    CYG_INVARIANT_CLASSOC(CdlListExpressionBody, original);

    expression_string           = original.expression_string;

    // These copy across the pointers
    data        = original.data;
    ranges      = original.ranges;

    // And this clears out the pointers, but leaves the expression objects lying around
    CdlListExpression tmp = const_cast<CdlListExpression>(&original);
    tmp->data.clear();
    tmp->ranges.clear();

    cdllistexpressionbody_cookie = CdlListExpressionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Destruction                              

CdlListExpressionBody::~CdlListExpressionBody()
{
    CYG_REPORT_FUNCNAME("CdlListExpression:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdllistexpressionbody_cookie        = CdlListExpressionBody_Invalid;
    expression_string                   = "";

    for (std::vector<CdlExpression>::iterator i = data.begin(); i != data.end(); i++) {
        delete *i;
        *i = 0;
    }
    for (std::vector<std::pair<CdlExpression, CdlExpression> >::iterator j = ranges.begin(); j != ranges.end(); j++) {
        delete j->first;
        delete j->second;
        j->first = 0;
        j->second = 0;
    }
    data.clear();
    ranges.clear();
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  check_this()                             

// ----------------------------------------------------------------------------
bool
CdlListExpressionBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlListExpressionBody_Magic != cdllistexpressionbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    switch(zeal) {
      case cyg_system_test      :
      case cyg_extreme          :
      case cyg_thorough         :
      {
          for (std::vector<CdlExpression>::const_iterator i = data.begin(); i != data.end(); i++) {
              if (!(*i)->check_this(cyg_quick)) {
                  return false;
              }
          }
          for (std::vector<std::pair<CdlExpression,CdlExpression> >::const_iterator j = ranges.begin();
               j != ranges.end();
               j++) {
              if (!(j->first->check_this(cyg_quick)) || !(j->second->check_this(cyg_quick))) {
                  return false;
              }
          }
      }
      case cyg_quick            :
      case cyg_trivial          :
      case cyg_none             :
      default                   :
          break;
    }

    return true;
}

//}}}
//{{{  Parsing                                  

// ----------------------------------------------------------------------------
// Parsing a list expression involves repeated parsing of ordinary
// expressions until an EOD token is reached.

CdlListExpression
CdlListExpressionBody::parse(std::string data)
{
    CYG_REPORT_FUNCNAMETYPE("CdlListExpression::parse", "result %p");

    // Allocate an expression object that can then be filled in.
    CdlListExpression result = new CdlListExpressionBody;

    // Do the parsing in a try/catch statement to make sure the
    // allocated expression gets freed on a parse error.
    try {
        int             index           = 0;
        int             end_index       = 0;
        CdlExprOp       op              = CdlExprOp_Invalid;
        CdlExpression   expr1           = 0;

        do {
            // Try to parse the next expression in the list
            op    = CdlExprOp_Invalid;
            expr1 = CdlExpressionBody::parse(data, index, op, end_index);

            // There should now be a valid expression, failure would have
            // resulted in an exception.
            CYG_ASSERT_CLASSC(expr1);

            // Allow for ranges.
            if (CdlExprOp_Range != op) {
                // A simple expression, just add it to the current data vector
                // "index" will contain the appropriate value.
                result->data.push_back(expr1);
            } else {
                // A range expression. Get the other end of the range.
                // This requires manipulating index a bit.
                CdlExpression expr2 = 0;
                index = end_index;
                op    = CdlExprOp_Invalid;
                try {
                    expr2 = CdlExpressionBody::parse(data, index, op, end_index);
                }
                catch (...) {
                    delete expr1;
                    throw;
                }
                result->ranges.push_back(std::make_pair(expr1, expr2));
            }
        } while (CdlExprOp_EOD != op);
    }
    catch (...) {
        delete result;
        throw;
    }

    // Keep track of the original string for diagnostics purposes
    result->expression_string = data;
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  update()                                 

// ----------------------------------------------------------------------------
// This code is invoked when it is necessary to update the references
// for the list expression. There are four situations in which this
// can happen: the package has just been loaded; the package is being
// unloaded; a referenced target is being created; a referenced target is
// being destroyed.
//
// The first two cases simply involve processing every expression that
// makes up the overall list expression. The last two cases involve
// searching through the expressions until an applicable one is found.
// Note that an expression may contain multiple references to another
// object, resulting in multiple calls to this function.

bool
CdlListExpressionBody::update(CdlTransaction transact, CdlNode source, CdlProperty source_prop, CdlNode dest, CdlUpdate change)
{
    CYG_REPORT_FUNCNAMETYPE("CdlListExpression::update", "result %d");
    CYG_REPORT_FUNCARG6XV(this, transact, source, source_prop, dest, change);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(source);
    CYG_PRECONDITION_CLASSC(source_prop);

    bool result = false;

    if ((CdlUpdate_Loaded == change) || (CdlUpdate_Unloading == change)) {

        std::vector<CdlExpression>::const_iterator expr_i;
        std::vector<std::pair<CdlExpression, CdlExpression> >::const_iterator pair_i;

        for (expr_i = data.begin(); expr_i != data.end(); expr_i++) {
            bool handled = (*expr_i)->update(transact, source, source_prop, dest, change);
            CYG_ASSERTC(handled);
            CYG_UNUSED_PARAM(bool, handled);
        }
        for (pair_i = ranges.begin(); pair_i != ranges.end(); pair_i++) {
            bool handled = pair_i->first->update(transact, source, source_prop, dest, change);
            CYG_ASSERTC(handled);
            handled = pair_i->second->update(transact, source, source_prop, dest, change);
            CYG_ASSERTC(handled);
        }
        
        result = true;
        
    } else {
        CYG_ASSERTC((CdlUpdate_Created == change) || (CdlUpdate_Destroyed == change));

        std::vector<CdlExpression>::const_iterator expr_i;
        std::vector<std::pair<CdlExpression, CdlExpression> >::const_iterator pair_i;

        for (expr_i = data.begin(); !result && (expr_i != data.end()); expr_i++) {
            result = (*expr_i)->update(transact, source, source_prop, dest, change);
        }
        for (pair_i = ranges.begin(); !result && (pair_i != ranges.end()); pair_i++) {
            result = pair_i->first->update(transact, source, source_prop, dest, change);
            if (!result) {
                result = pair_i->second->update(transact, source, source_prop, dest, change);
            }
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Evaluation                               

// ----------------------------------------------------------------------------
// Evaluation. The hard work is actually done in eval_internal()

void
CdlListExpressionBody::eval(CdlEvalContext& context, CdlListValue& result)
{
    CYG_REPORT_FUNCNAME("CdlListExpression::eval");
    CYG_REPORT_FUNCARG3XV(this, &context, &result);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSOC(context);

    this->eval_internal(context, result);

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// This requires evaluating each expression in the data and ranges
// vectors and adding the result to the appropriate vector in result.
// Various error conditions are possible.

void
CdlListExpressionBody::eval_internal(CdlEvalContext& context, CdlListValue& result)
{
    CYG_REPORT_FUNCNAME("CdlListExpression::eval_internal");
    CYG_REPORT_FUNCARG2XV(this, &context);

    result.table.clear();
    result.integer_ranges.clear();
    result.double_ranges.clear();

    CdlSimpleValue val1;
    CdlSimpleValue val2;

    try {
        for (std::vector<CdlExpression>::const_iterator i = data.begin(); i != data.end(); i++) {
            (*i)->eval_internal(context, val1);
            if ("" != val1.get_value()) {
              result.table.push_back(val1);
            }
        }
        for (std::vector<std::pair<CdlExpression,CdlExpression> >::const_iterator j = ranges.begin(); j != ranges.end(); j++) {
            j->first->eval_internal(context, val1);
            j->second->eval_internal(context, val2);

            if (val1.has_integer_value() && val2.has_integer_value()) {
                cdl_int x1 = val1.get_integer_value();
                cdl_int x2 = val2.get_integer_value();
                if (x1 > x2) {
                    cdl_int tmp = x1;
                    x1 = x2;
                    x2 = tmp;
                }
                result.integer_ranges.push_back(std::make_pair(x1, x2));
            } else if (val1.has_double_value() && val2.has_double_value()) {
                double x1 = val1.get_double_value();
                double x2 = val2.get_double_value();
                if (x1 > x2) {
                    double tmp = x1;
                    x1 = x2;
                    x2 = tmp;
                }
                result.double_ranges.push_back(std::make_pair(x1, x2));
            } else {
                throw CdlEvalException("Range expression involves non-numerical limits");
            }
        }
        
        // Any problems would have resulted in an exception. If there
        // was a previous EvalExeption for this property, it is no
        // longer applicable
        if ((0 != context.transaction) && (0 != context.node) && (0 != context.property)) {
            context.transaction->clear_conflicts(context.node, context.property, &CdlConflict_EvalExceptionBody::test);
        }
        
    } catch(CdlEvalException e) {

        if ((0 != context.transaction) && (0 != context.node) && (0 != context.property)) {
            
            CdlConflict conflict = context.transaction->get_conflict(context.node, context.property,
                                                                              &CdlConflict_EvalExceptionBody::test);
            if (0 == conflict) {
                CdlConflict_EvalExceptionBody::make(context.transaction, context.node, context.property, e.get_message());
            } else {
                CdlConflict_EvalException eval_conf = dynamic_cast<CdlConflict_EvalException>(conflict);
                CYG_ASSERTC(0 != eval_conf);
                if (eval_conf->get_explanation() != e.get_message()) {
                
                    // Replace the conflict object. Higher level will be informed about this.
                    context.transaction->clear_conflict(conflict);
                    CdlConflict_EvalExceptionBody::make(context.transaction, context.node, context.property, e.get_message());
                }
            }
        }

        throw;
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  is_member()                              

// ----------------------------------------------------------------------------

bool
CdlListExpressionBody::is_member(CdlEvalContext& context, CdlSimpleValue& val)
{
    CYG_REPORT_FUNCNAMETYPE("CdlListExpression::is_member (value)", "result %d");
    CYG_REPORT_FUNCARG3XV(this, &context, &val);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSOC(context);

    bool result = false;
    CdlListValue list_val;
    eval_internal(context, list_val);
    result = list_val.is_member(val);

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlListExpressionBody::is_member(CdlEvalContext& context, std::string val)
{
    CYG_REPORT_FUNCNAMETYPE("CdlListExpression::is_member (string)", "result %d");
    CYG_REPORT_FUNCARG2XV(this, &context);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSOC(context);

    bool result = false;
    CdlListValue list_val;
    eval_internal(context, list_val);
    result = list_val.is_member(val);

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlListExpressionBody::is_member(CdlEvalContext& context, cdl_int val)
{
    CYG_REPORT_FUNCNAMETYPE("CdlListExpression::is_member (int)", "result %d");
    CYG_REPORT_FUNCARG3XV(this, &context, (int) val);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSOC(context);

    bool result = false;
    CdlListValue list_val;
    eval_internal(context, list_val);
    result = list_val.is_member(val);

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlListExpressionBody::is_member(CdlEvalContext& context, double val)
{
    CYG_REPORT_FUNCNAMETYPE("CdlListExpression::is_member (double)", "result %d");
    CYG_REPORT_FUNCARG2XV(this, &context);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSOC(context);

    bool result = false;
    CdlListValue list_val;
    eval_internal(context, list_val);
    result = list_val.is_member(val);

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Misc                                     

// ----------------------------------------------------------------------------

std::string
CdlListExpressionBody::get_original_string() const
{
    CYG_REPORT_FUNCNAME("CdlListExpression::get_original_string");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return expression_string;
}

//}}}

//}}}
//{{{  CdlGoalExpression                

// ----------------------------------------------------------------------------
// Constructors etc. are pretty much as per ordinary and list
// expressions. Most of the work is done in the private base class.

CdlGoalExpressionBody::CdlGoalExpressionBody()
    : CdlExpressionBody()
{
    CYG_REPORT_FUNCNAME("CdlGoalExpression::default_constructor");
    CYG_REPORT_FUNCARG1XV(this);

    expression_string           = "";
    cdlgoalexpressionbody_cookie = CdlGoalExpressionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlGoalExpressionBody::CdlGoalExpressionBody(const CdlGoalExpressionBody& original)
    : CdlExpressionBody(original)
{
    CYG_REPORT_FUNCNAME("CdlGoalExpression:: copy constructor");
    CYG_REPORT_FUNCARG2XV(this, &original);
    CYG_INVARIANT_CLASSOC(CdlGoalExpressionBody, original);

    expression_string           = original.expression_string;
    cdlgoalexpressionbody_cookie = CdlGoalExpressionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlGoalExpressionBody::~CdlGoalExpressionBody()
{
    CYG_REPORT_FUNCNAME("CdlGoalExpression:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlgoalexpressionbody_cookie = CdlGoalExpressionBody_Invalid;
    expression_string            = "";
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Parsing. A goal expression acts a bit like a list expression with
// implicit && operators between the various expressions. It could be
// implemented as a vector of expressions (which might make diagnostics
// easier) but it is almost as easy to derive a goal expression from
// an ordinary one.

CdlGoalExpression
CdlGoalExpressionBody::parse(std::string data)
{
    CYG_REPORT_FUNCNAMETYPE("CdlGoalExpression::parse", "result %p");

    CdlGoalExpression result = new CdlGoalExpressionBody;

    try {
        int       index         = 0;
        CdlExprOp op            = CdlExprOp_Invalid;
        int       end_index     = 0;

        // Parse the first expression in the data.
        CdlExpressionBody::continue_parse(result, data, index, op, end_index);

        // At this stage we have reached end-of-data or we should be
        // at the start of another expression - any binary or ternary
        // operands would have been subsumed in the previous expression.
        // We need to keep adding && operators and new expressions until
        // end-of-data.
        while (CdlExprOp_EOD != op) {
            op = CdlExprOp_And;
            CdlExpressionBody::continue_parse(result, data, index, op, end_index);
        }
    }
    catch(...) {
        delete result;
        throw;
    }

    // Keep track of the original expression string for diagnostics purposes
    result->expression_string = data;
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
void
CdlGoalExpressionBody::eval(CdlEvalContext& context, bool& result)
{
    CYG_REPORT_FUNCNAME("CdlGoalExpression::eval");
    CYG_REPORT_FUNCARG2XV(this, &context);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSOC(context);

    eval_internal(context, result);

    CYG_REPORT_RETURN();
}

bool
CdlGoalExpressionBody::eval(CdlEvalContext& context)
{
    CYG_REPORT_FUNCNAMETYPE("CdlGoalExpression::eval", "result %d");
    CYG_REPORT_FUNCARG2XV(this, &context);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSOC(context);

    bool result;
    eval_internal(context, result);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Provide access to the underlying CdlExpression object. This allows the
// inference engine etc. to work out why a goal expression is failing

CdlExpression
CdlGoalExpressionBody::get_expression()
{
    CYG_REPORT_FUNCNAMETYPE("CdlGoalExpression::get_expression", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlExpression result = this;
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------

bool
CdlGoalExpressionBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlGoalExpressionBody_Magic != cdlgoalexpressionbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    // There is no data specific to a goal expression, just let the
    // underlying check_this() member do its stuff.
    
    return inherited::check_this(zeal);
}

// ----------------------------------------------------------------------------

std::string
CdlGoalExpressionBody::get_original_string() const
{
    CYG_REPORT_FUNCNAME("CdlGoalExpression::get_original_string");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return expression_string;
}

// ----------------------------------------------------------------------------

void
CdlGoalExpressionBody::eval_internal(CdlEvalContext& context, bool& result)
{
    CYG_REPORT_FUNCNAME("CdlGoalExpression::eval_internal");
    CYG_REPORT_FUNCARG2XV(this, &context);
    // The assertions are all done in the calling code

    // Start by evaluating the underlying expression
    CdlSimpleValue      val;
    try {
        inherited::eval_internal(context, val);

        // The evaluation succeeded. Do we have an integer, a string, ...?
        if (val.has_integer_value()) {
            result = (0 != val.get_integer_value());
        } else if (val.has_double_value()) {
            result = (0.0 != val.get_double_value());
        } else {
            result = ("" != val.get_value());
        }

        // If there is an EvalException conflict for this property, it is no longer applicable
        if ((0 != context.transaction) && (0 != context.node) && (0 != context.property)) {
            context.transaction->clear_conflicts(context.node, context.property,
                                                          &CdlConflict_EvalExceptionBody::test);
        }
        
    } catch(CdlEvalException e) {
        if ((0 != context.transaction) && (0 != context.node) && (0 != context.property)) {
            CdlConflict conflict = context.transaction->get_conflict(context.node, context.property,
                                                                          &CdlConflict_EvalExceptionBody::test);
            if (0 == conflict) {
                CdlConflict_EvalExceptionBody::make(context.transaction, context.node, context.property, e.get_message());
            } else {
                CdlConflict_EvalException eval_conf = dynamic_cast<CdlConflict_EvalException>(conflict);
                CYG_ASSERTC(0 != eval_conf);
                if (eval_conf->get_explanation() != e.get_message()) {
                    // Replace the conflict object. Higher level can detect this.
                    context.transaction->clear_conflict(conflict);
                    CdlConflict_EvalExceptionBody::make(context.transaction, context.node, context.property, e.get_message());
                }
            }
            throw;
        }
    }
    
    CYG_REPORT_RETURN();
}

//}}}
