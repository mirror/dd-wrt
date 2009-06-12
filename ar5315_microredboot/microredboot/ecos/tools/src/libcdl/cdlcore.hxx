#ifndef __CDLCORE_HXX
# define __CDLCORE_HXX

//{{{  Banner                                           

//==========================================================================
//
//      cdlcore.hxx
//
//      The core parts of the library. This header defines aspects of
//      CDL that are shared between software cdl, hcdl, scdl, and any
//      future languages based on the same core technology.
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002 Bart Veer
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
//==========================================================================
//#####DESCRIPTIONBEGIN####                                             
//
// Author(s):           bartv
// Contributors:        bartv
// Date:                1999-04-15
//
//####DESCRIPTIONEND####
//==========================================================================

//}}}
//{{{  Platform dependencies                            

// ----------------------------------------------------------------------------
// Visual C++ has the delightful feature that the source browser will generate
// warnings if there are any identifiers of length >= 256 characters, while at
// the same time use of templates in the standard C++ library can easily
// generate functions that long. It appears that the only way to disable the
// warnings is by use of a %$#@(%*&%! #pragma.
//
// Similarly, VC++ gives spurious warnings when it comes to multiple virtual
// inheritance.
#ifdef _MSC_VER
# pragma warning( disable: 4786 )
# pragma warning( disable: 4250 )
#endif

//}}}
//{{{  nested #include's                                

// ----------------------------------------------------------------------------
// The libcdl API is defined using parts of the standard C++ library,
// including strings and various bits of STL. Therefore these headers must
// be #include'd here for the header file to work.
#include <vector>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <string>
#include <functional>
#include <algorithm>

// <cctype> is needed in various places in the implementation.
// This #include should be moved to an implementation-specific
// header.
#include <cctype>

// Now for some eCos host-side infrastructure headers.
//
// Get the cyg_int64 data type and CYG_UNUSED_PARAM() macro.
#include <cyg/infra/cyg_type.h>

// Some of the classes need to reference the cyg_assert_class_zeal enum.
// Also inline functions may perform assertions.
#include <cyg/infra/cyg_ass.h>

// This header file also depends on having a suitable Tcl installation
// Unfortunately <tcl.h> does some ugly things in the interests of
// portability, including defining symbols such as EXTERN when
// necessary, and this has to be patched up here as cleanly as possible.
#ifndef CONST
# define __CDL_CONST_UNDEFINED
#endif
#ifndef EXTERN
# define __CDL_EXTERN_UNDEFINED
#endif
#ifndef VOID
# define __CDL_VOID_UNDEFINED
#endif
#ifndef CHAR
# define __CDL_CHAR_UNDEFINED
#endif
#ifndef SHORT
# define __CDL_SHORT_UNDEFINED
#endif
#ifndef LONG
# define __CDL_LONG_UNDEFINED
#endif

extern "C" {
#include <tcl.h>
}

#ifdef __CDL_CONST_UNDEFINED
# undef CONST
# undef __CDL_CONST_UNDEFINED
#endif
#ifdef __CDL_EXTERN_UNDEFINED
# undef EXTERN
# undef __CDL_EXTERN_UNDEFINED
#endif
#ifdef __CDL_VOID_UNDEFINED
# undef VOID
# undef __CDL_VOID_UNDEFINED
#endif
#ifdef __CDL_CHAR_UNDEFINED
# undef CHAR
# undef __CDL_CHAR_UNDEFINED
#endif
#ifdef __CDL_SHORT_UNDEFINED
# undef SHORT
# undef __CDL_SHORT_UNDEFINED
#endif
#ifdef __CDL_LONG_UNDEFINED
# undef LONG
# undef __CDL_LONG_UNDEFINED
#endif

//}}}

//{{{  Primitive types, constants:, enums, etc.         

// ----------------------------------------------------------------------------
// The CDL languages are defined in terms of arbitrary precision
// arithmetic. This is necessary to allow e.g. pointers to be
// manipulated at the CDL level on 64 bit target processors.
//
// Temporarily it is not necessary to provide this precision, so it is
// convenient to stick to 64 bit integers as provided by the
// underlying infrastructure. However the API is defined in terms of
// the type cdl_int, so that it will be easier in future to make the
// change to the correct datatype. At that point cdl_int can be
// redefined to be a class which supports the appropriate operators.

typedef cyg_int64 cdl_int;

// ---------------------------------------------------------------------------
// A common concept in the CDL language is a small amount of TCL code.
// This is currently stored as a simple string. Conceivably it could
// be byte-compiled and stored accordingly.

typedef std::string  cdl_tcl_code;

// ----------------------------------------------------------------------------
// CDL values.
//
// CDL is a declarative programming language. It does involve the
// manipulation of values, but such values do not necessarily
// correspond to hardware-level entities such as integers or double
// precision numbers. Hence the term "type" is avoided, "flavor"
// is used instead. CDL understands four different flavors.
//
//    None  |  Bool
//  --------+--------
//    Data  |BoolData
//
//
// The flavor "none" is used for entities that serve only as
// placeholders in the hierarchy, allowing other entities to be
// grouped more easily.
//
// Boolean entities can be either enabled or disabled. This is the
// most common flavor for software configuration options, the user can
// either enable or disable some unit of functionality. For software
// packages implemented in C or C++ the implementation is obvious: iff
// the entity is enabled then there will be a #define, and code will
// check the setting using e.g. #ifdef.
//
// The flavor "data" implies some arbitrary data. Internally this will
// be held as a string. Other properties such as legal_values,
// check_proc and entry_proc can be used to constrain the
// actual values, for example to an integer value within a certain
// range.
//
// The flavor "booldata" combines the previous two: it means that
// the option can be either enabled or disabled, and if it is
// enabled then it must have a value as per legal_values etc.
// One example of this is a software package: this may be either
// enabled or disabled, and if it is enabled then it has a value
// corresponding to the version string. Another example is a hardware
// pin: this may or may not be connected, and if it is connected
// then its value identifies some other pin.
//
// An entity's flavor is not always sufficient by itself to specify
// how the user can manipulate it in a graphical tool. Obviously an
// entity of flavor "none" cannot be manipulated at all. Flavor "bool"
// normally implies a checkbutton, but occasionally a radiobutton will
// be more appropriate. "Data" says very little about the user
// interaction, it will be necessary to examine other properties such
// as legal_values to determine a sensible representation. The same
// goes for "BoolData", with the additional possibility that the
// entity may be disabled.
//
// It can be argued that three of the flavors are redundant: both Bool
// and BoolData could be implemented as cases of "Data" with a special
// legal value "disabled" (or false, or whatever); "None" could be
// implemented as constant "Data"; effectively CDL would manipulate
// all data as strings, just like e.g. all variables in Tcl, or just
// like all scalars in Perl. This approach is certainly tempting and
// might well make it easier to document the language, but in practice
// it would result in more verbose CDL: boolean entities really are a
// different beast from data entities.
//
// It can also be argued that there should be more flavors. For
// example there could be separate flavors for integer data, floating
// point data, string data, and so on. There are a number of good
// reasons for not doing so:
//
// 1) applying separate constraints such as legal_values allows much
//    finer control over the actual values, for example numbers within a
//    given range. As likely as not, a value will be constrained to
//    something smaller than the range MININT to MAXINT (whatever those
//    happen to be for the current target).
//
// 2) where do you stop? Do you provide separate flavors for signed
//    vs. unsigned? Char, wchar_t, short, int, long, long long? How about
//    the eCos data types cyg_ucount8, cyg_uint8, ... Is there support
//    for enums? Arrays? Bitfields? Structures? Unions? C++ classes?
//    How about other programming languages such as Ada or Java?
//
//    Any attempt to implement a grand union of all data types in CDL 
//    is doomed to failure and should not be attempted. Treating
//    everything as a string instead has proven successful in a number
//    of languages, including Tcl and Perl.
//
// 3) for some variants of CDL, for example hardware CDL, it may not
//    make much sense to display a value directly and allow it to be
//    manipulated directly. The value associated with a pin entity
//    identifies the pin to which it is connected, and typically
//    this value will be manipulated by drag and drop rather than by
//    typing some characters. Such a value certainly does not correspond
//    to any machine data type.
//
// Another reason for extending the number of flavors is to provide
// more information. For example there could be a specialized version
// of the boolean flavor called "radio". This would imply a specific
// representation in the user interface, and it would also impose
// a constraint that it implicitly precludes any other radio entities
// within the same group. However the same information can be specified
// by other more general means such as requires statements.

enum CdlValueFlavor {
    CdlValueFlavor_Invalid      =  0,
    CdlValueFlavor_None         =  1,
    CdlValueFlavor_Bool         =  2,
    CdlValueFlavor_BoolData     =  3,
    CdlValueFlavor_Data         =  4
};


// Another important aspect of a value is where it came from. There
// are a number of possible sources: the default value, calculated
// from a default_value property; a value inferred by the inference
// engine; a value set by a wizard; and a value set explicitly by
// the user. These sources have different priorities, so for example
// the inference engine can safely replace a calculated default
// value without prompting the user, but changing a user-set value
// automatically is undesirable.
//
// Wizard-generated values are considered more valuable than default
// or inferred values (there is some user input involved), but less
// valuable than values set explicitly by the user: the idea is that
// a wizard asks fairly generic questions and makes a best guess at
// the correct values, which may not be precise enough for the
// user's needs.
//
// Arguably dialogs provide a level between wizards and users, in that
// a dialog can theoretically manipulate several entities in one go so
// it is a less precise way of setting values. At this stage it does
// not seem worthwhile to add this distinction.
//
// The library actually maintains separate values for each source,
// as well as the current source which is what actually gets used.
// In theory it is possible for the user interface code to let
// the user switch between these. It is not yet clear whether this
// makes sense from an end user's perspective.

enum CdlValueSource {
    CdlValueSource_Invalid              = -1, // 0 is needed for array indexing
    CdlValueSource_Default              =  0,
    CdlValueSource_Inferred             =  1,
    CdlValueSource_Wizard               =  2,
    CdlValueSource_User                 =  3,
    CdlValueSource_Current              =  4
};        

// ----------------------------------------------------------------------------
// Update support.
//
// When there is a change to a node and there are references to that node,
// the referencing properties will want to be informed about this. There
// are various different kinds of changes, not all of which are always
// relevant. For example, if a CDL entity gets destroyed or unloaded then
// all referencing entities are likely to want to know about this, but
// if a container's value changes then this has no effect on a reference
// in e.g. a "parent" property. In some cases it is also useful to apply
// updates to nodes rather than properties, e.g. when a node becomes
// active or inactive.
//
// The generic update code is also used for initialization and finalization,
// i.e. when the source object itself has just been loaded or is
// being unloaded.
//
// For any particular update at most one bit set, but it is often
// appropriate to treat several different kinds of update with
// common code. Hence the enum values can be or'ed and and'ed.

enum CdlUpdate {
    CdlUpdate_Loaded            = 0x0001,       // The source has just been loaded
    CdlUpdate_Init              = 0x0002,       // Second-phase of a load operation
    CdlUpdate_Unloading         = 0x0004,       // The source is being unloaded
    CdlUpdate_Created           = 0x0008,       // The destination has just been created
    CdlUpdate_Destroyed         = 0x0010,       // The destination is being destroyed
    CdlUpdate_ValueChange       = 0x0020,       // The destination's value has changed.
                                                // This gets applied to nodes as well                   
    CdlUpdate_ActiveChange      = 0x0040        // The node has become active or inactive
};

// ----------------------------------------------------------------------------
// Inference engine callback.
//
// During a transaction there may be one or more invocations of the inference
// engine, followed by a callback which should display the current transaction
// status to the user and allow one or more recommended fixes to be accepted.
// The callback's return code indicates what should happen next. "Cancel"
// is pretty obvious. "Continue" may result in a commit, or it may result in
// another iteration.

enum CdlInferenceCallbackResult {
    CdlInferenceCallbackResult_Continue = 0x01,
    CdlInferenceCallbackResult_Cancel   = 0x02
};

// ----------------------------------------------------------------------------
// Widget hints.
//
// The library can provide a hint to the GUI code as to a sensible
// widget to use for displaying a particular valuable. There are separate
// hints for the bool and data parts.

enum CdlBoolWidget {
    CdlBoolWidget_None                  = 0,    // The boolean part is not applicable
    CdlBoolWidget_CustomDialog          = 1,    // There is a valid custom dialog property
    CdlBoolWidget_CheckButton           = 2,    // For simple booleans
    CdlBoolWidget_Radio                 = 3,    // For several mutual exclusive options,
                                                // the data structure will provide a string identifier
};

enum CdlValueWidget {
    CdlValueWidget_None                 = 0,    // The value part is not applicable
    CdlValueWidget_CustomDialog         = 1,    // There is a valid custom dialog property
    CdlValueWidget_Loadable             = 2,    // Use package/version dialog
    CdlValueWidget_EntryBox             = 3,    // Fallback
    CdlValueWidget_MultilineString      = 4,    // For complicated strings
    CdlValueWidget_DecimalRange         = 5,    // e.g. 1 to 16
                                                // Could be implemented as scale, radio buttons, entry, pull-down menu,
                                                // combo box, ... depending on GUI conventions and number of entries
    CdlValueWidget_HexRange             = 6,    // e.g. 0x01 to 0x10
    CdlValueWidget_OctalRange           = 7,    // e.g. 01 to 020
    CdlValueWidget_DoubleRange          = 8,    // e.g. 0.1 to 0.2
    CdlValueWidget_NumericSet           = 9,    // e.g. 1 2 4 8 16
                                                // The exact nature of the numbers is irrelevant, they will only
                                                // get displayed, not edited
                                                // Could be implemented as radio buttons, entry widget, pull-down menu,
                                                // combo box, ... depending on GUI conventions and number of entries
                                                // Each entry can have its own representation
    CdlValueWidget_StringSet            = 10    // e.g. "ram", "rom"

    // More to be added, e.g. for compiler flag handling
};

// ----------------------------------------------------------------------------
// Value formats.
//
// The CDL input data can accept numbers in a variety of formats,
// for example hexadecimal as well as decimal. It is desirable to try
// to keep track of this formatting information where possible, so
// that what the user sees and what ends up in header files corresponds
// more closely to what is in the raw CDL data. For example, it is
// much easier to understand 0x7fffffff than its decimal equivalent.
//
// The information kept here is very imprecise, it provides only
// minimal formatting information. It is not clear yet whether this
// will suffice or whether something more exact is going to be needed.
enum CdlValueFormat
{
    CdlValueFormat_Default              = 0,
    CdlValueFormat_Hex                  = 1,
    CdlValueFormat_Octal                = 2
};

//}}}
//{{{  Exception classes                                

// ----------------------------------------------------------------------------
// Some parts of the library make use of C++ exception handling. A number
// of exception classes related to this library are useful. In addition
// just about every part of the library can throw std::bad_alloc, but this
// is not checked for explicitly anywhere.

// This class is used for all exceptions where an error message should
// be displayed to the user. There is a single string message associated
// with the exception.

class CdlStringException {
    friend class CdlTest;

  public:
    CdlStringException(std::string message_arg) {
        message = message_arg;
    }
    CdlStringException(const CdlStringException& original) {
        message = original.message;
    }
    CdlStringException& operator=(const CdlStringException& original) {
        message = original.message;
        return *this;
    }
    ~CdlStringException() {
        message = "";
    }
    const std::string& get_message() const {
        return message;
    }
  private:
    std::string message;
    CdlStringException();
};

// CdlInputOutputException: this gets thrown when something goes wrong during
// file I/O operations, e.g. a file exists but cannot be opened. The
// exception contains a simple string explaining the error. This string
// may contain multiple lines, it is intended to be written to stderr
// or displayed in either a text widget or a dialog box.
//
// A separate class rather than a typedef is used to avoid any possible
// error message confusion. Everything gets inlined so there should be
// no performance issues.

class CdlInputOutputException : public CdlStringException {
    friend class CdlTest;
  public:
    CdlInputOutputException(std::string message_arg) :
        CdlStringException(message_arg) {
    }
    CdlInputOutputException(const CdlInputOutputException& original) :
        CdlStringException(original) {
    }
    CdlInputOutputException& operator=(const CdlInputOutputException& original) {
        (void) CdlStringException::operator=(original);
        return *this;
    }
};

// This class is used when any parsing happens at the C++ level rather
// than at the Tcl level. The exception should be caught before it
// propagates through the Tcl interpreter, or the latter will end up
// in an inconsistent state.

class CdlParseException : public CdlStringException {
    friend class CdlTest;
  public:
    CdlParseException(std::string message_arg) :
        CdlStringException(message_arg) {
    }
    CdlParseException(const CdlParseException& original) :
        CdlStringException(original) {
    }
    CdlParseException& operator=(const CdlParseException& original) {
        (void) CdlStringException::operator=(original);
        return *this;
    }
};

// Evaluating an expression may fail for a variety of reasons, e.g. because
// some referenced entity has not been loaded into the configuration.
// This exception can be thrown in such cases.

class CdlEvalException : public CdlStringException {
    friend class CdlTest;
  public:
    CdlEvalException(std::string message_arg) :
        CdlStringException(message_arg) {
    }
    CdlEvalException(const CdlEvalException& original) :
        CdlStringException(original) {
    }
    CdlEvalException& operator=(const CdlEvalException& original) {
        (void) CdlStringException::operator=(original);
        return *this;
    }
};

//}}}
//{{{  Forward declarations of the body classes         

// ----------------------------------------------------------------------------
// This section provides forward declarations of the main classes in
// the core of the library. Each variant of CDL will define additional
// classes, e.g. cdl_option, but these will usually be derived from
// the core ones.

// There are three types of expression in CDL:
// 1) ordinary expressions evaluate to a single value. The most common
//    use is for the legal_values property.
// 2) list expressions evaluate to a range of values, e.g. 1 to 10,
//    and the most common use is for the legal_values property.
// 3) goal expressions evaluate to either true or false and are used
//    for e.g. requires and active_if properties.
class CdlExpressionBody;
class CdlListExpressionBody;
class CdlGoalExpressionBody;

// There are also objects for simple values, values and list values.
// These are expanded classes, there are no associated pointer
// types. It is quite likely that values need to be copied around
// on the stack.
class CdlSimpleValue;
class CdlValue;
class CdlListValue;

// Properties. The base class is CdlProperty, and there are a number
// of derived classes provided as standard. Additional derived classes
// may be added in future.
class CdlPropertyBody;
class CdlProperty_MinimalBody;
class CdlProperty_StringBody;
class CdlProperty_TclCodeBody;
class CdlProperty_ReferenceBody;
class CdlProperty_StringVectorBody;
class CdlProperty_ExpressionBody;
class CdlProperty_ListExpressionBody;
class CdlProperty_GoalExpressionBody;

// Base classes. CDL entities such as options and components derive
// from one or more of these, using virtual inheritance.
// 
// The lowest-level class is CdlNodeBody.
//
// 1) a node usually lives in a hierarchy, below a toplevel
//    and with a container object as the parent. However nodes
//    can live outside a container on a temporary basis,
//    and toplevel objects have no parent.
//
// 2) a node has a name that is unique within the hierarchy.
//
// 3) a node has a vector of properties. Actually some entities
//    will have an empty vector, e.g. the orphans container
//    that is internal to the library. However it is too
//    inconvenient to have separate base classes for these.
//
// 4) nodes can be referred to by properties in other nodes.
class CdlNodeBody;

// A container is a node that can contain other nodes.
class CdlContainerBody;

// A loadable object is a container whose data has come out of a CDL
// script of some sort. It also stores details about all entities that
// were loaded via this script (even if some of them were reparented)
// thus supporting unload operations.
class CdlLoadableBody;

// A toplevel object is a container that acts as the toplevel of
// a hierarchy, in other words its parent is always 0. In addition
// a toplevel keeps track of all the names used in the hierarchy,
// thus facilitating navigation.
class CdlToplevelBody;

// The remaining classes all add functionality to CdlNode, directly or
// indirectly.
//
// A user-visible object is likely to appear in the user interface.
// This means it may have an alias string, a description, a
// documentation URL, and a gui_hint field.
class CdlUserVisibleBody;

// A valuable object has a value that can be retrieved but not
// necessarily modified by the user. For example the value of an
// interface is always calculated and users can never change it.
// Valuable objects have a whole bunch of associated properties
// including dependencies.
class CdlValuableBody;

// A parentable object has the parent property, i.e. it can
// be reparented to anywhere in the hierarchy
class CdlParentableBody;

// A buildable object is a valuable object that may result in
// something being built, typically a library in the case of
// software packages.
class CdlBuildableBody;

// A loadable that contains buildables
class CdlBuildLoadableBody;

// A definable object is a valuable object whose value can result
// in #define statements in a header file
class CdlDefinableBody;

// A loadable which can contain definables
class CdlDefineLoadableBody;

// TODO: add instantiation support

// Custom dialogs and wizards are provided by the core.
class CdlDialogBody;
class CdlWizardBody;
class CdlInterfaceBody;

// Support for Tcl interpreters is also in the core, since it is
// difficult to do anything CDL-related without at least one Tcl
// interpreter lying around.
class CdlInterpreterBody;

// The basic conflict class is part of the core library, as are a
// number of common derived classes for specific types of conflict.
class CdlConflictBody;
class CdlConflict_UnresolvedBody;
class CdlConflict_IllegalValueBody;
class CdlConflict_EvalExceptionBody;
class CdlConflict_RequiresBody;
class CdlConflict_DataBody;

// Many operations happen (or may happen) in the context of a
// transaction. This is necessary to keep track of the various
// changes that can happen: for example, changing a component's
// value may require other entities' default values to be
// recalculated; it may change some legal_values list expressions,
// causing current values to become invalid; it may affect
// "requires" properties, causing goals to become satisfied or
// not-satisfied; it may change the "active" state of everything
// below the component, not to mention any entity with an
// "active_if" properties, and when an entity becomes active or
// inactive that may in turn affect other entities.
//
// Keeping track of all of this via recursion is possible, but there
// are problems. If an entity is updated multiple times, no
// optimizations are possible. It becomes much more difficult to
// detect cycles. During an unload operation things can get very
// messy. There is no easy way to track all of the changes and report
// them to higher level code via a callback. There is no support
// for any kind of rollback. A transaction model potentially
// provides support for all of this, at the cost of a more
// complex API.
class CdlTransactionBody;

// This class is used to pass information back to the application
// about what has actually changed in a transaction.
class CdlTransactionCallback;


// Build info class. This is always an expanded object, but is
// needed here to break a circular dependency.
class CdlBuildInfo;

// ----------------------------------------------------------------------------
// Typedefs for the pointers. There are separate typedefs to cope with
// const vs. non-const objects. Otherwise you end up with the problem
// that "const CdlNode x" means that the pointer is const, not the
// object pointed at.

typedef CdlExpressionBody*              CdlExpression;
typedef CdlListExpressionBody*          CdlListExpression;
typedef CdlGoalExpressionBody*          CdlGoalExpression;

typedef CdlPropertyBody*                CdlProperty;
typedef CdlProperty_MinimalBody*        CdlProperty_Minimal;
typedef CdlProperty_StringBody*         CdlProperty_String;
typedef CdlProperty_TclCodeBody*        CdlProperty_TclCode;
typedef CdlProperty_ReferenceBody*      CdlProperty_Reference;
typedef CdlProperty_StringVectorBody*   CdlProperty_StringVector;
typedef CdlProperty_ExpressionBody*     CdlProperty_Expression;
typedef CdlProperty_ListExpressionBody* CdlProperty_ListExpression;
typedef CdlProperty_GoalExpressionBody* CdlProperty_GoalExpression;

typedef CdlNodeBody*                    CdlNode;
typedef CdlContainerBody*               CdlContainer;
typedef CdlLoadableBody*                CdlLoadable;
typedef CdlToplevelBody*                CdlToplevel;
typedef CdlUserVisibleBody*             CdlUserVisible;
typedef CdlValuableBody*                CdlValuable;
typedef CdlParentableBody*              CdlParentable;
typedef CdlBuildableBody*               CdlBuildable;
typedef CdlBuildLoadableBody*           CdlBuildLoadable;
typedef CdlDefinableBody*               CdlDefinable;
typedef CdlDefineLoadableBody*          CdlDefineLoadable;

typedef CdlDialogBody*                  CdlDialog;
typedef CdlWizardBody*                  CdlWizard;
typedef CdlInterfaceBody*               CdlInterface;

typedef CdlInterpreterBody*             CdlInterpreter;

typedef CdlConflictBody*                CdlConflict;
typedef CdlConflict_UnresolvedBody*     CdlConflict_Unresolved;
typedef CdlConflict_IllegalValueBody*   CdlConflict_IllegalValue;
typedef CdlConflict_EvalExceptionBody*  CdlConflict_EvalException;
typedef CdlConflict_RequiresBody*       CdlConflict_Requires;
typedef CdlConflict_DataBody*           CdlConflict_Data;

typedef CdlTransactionBody*             CdlTransaction;

// ----------------------------------------------------------------------------

typedef const CdlExpressionBody*              CdlConstExpression;
typedef const CdlListExpressionBody*          CdlConstListExpression;
typedef const CdlGoalExpressionBody*          CdlConstGoalExpression;

typedef const CdlPropertyBody*                CdlConstProperty;
typedef const CdlProperty_MinimalBody*        CdlConstProperty_Minimal;
typedef const CdlProperty_StringBody*         CdlConstProperty_String;
typedef const CdlProperty_TclCodeBody*        CdlConstProperty_TclCode;
typedef const CdlProperty_ReferenceBody*      CdlConstProperty_Reference;
typedef const CdlProperty_StringVectorBody*   CdlConstProperty_StringVector;
typedef const CdlProperty_ExpressionBody*     CdlConstProperty_Expression;
typedef const CdlProperty_ListExpressionBody* CdlConstProperty_ListExpression;
typedef const CdlProperty_GoalExpressionBody* CdlConstProperty_GoalExpression;

typedef const CdlNodeBody*                    CdlConstNode;
typedef const CdlContainerBody*               CdlConstContainer;
typedef const CdlLoadableBody*                CdlConstLoadable;
typedef const CdlToplevelBody*                CdlConstToplevel;
typedef const CdlUserVisibleBody*             CdlConstUserVisible;
typedef const CdlValuableBody*                CdlConstValuable;
typedef const CdlParentableBody*              CdlConstParentable;
typedef const CdlBuildableBody*               CdlConstBuildable;
typedef const CdlBuildLoadableBody*           CdlConstBuildLoadable;
typedef const CdlDefinableBody*               CdlConstDefinable;
typedef const CdlDefineLoadableBody*          CdlConstDefineLoadable;

typedef const CdlDialogBody*                  CdlConstDialog;
typedef const CdlWizardBody*                  CdlConstWizard;
typedef const CdlInterfaceBody*               CdlConstInterface;

typedef const CdlInterpreterBody*             CdlConstInterpreter;

typedef const CdlConflictBody*                CdlConstConflict;
typedef const CdlConflict_UnresolvedBody*     CdlConstConflict_Unresolved;
typedef const CdlConflict_IllegalValueBody*   CdlConstConflict_IllegalValue;
typedef const CdlConflict_EvalExceptionBody*  CdlConstConflict_EvalException;
typedef const CdlConflict_RequiresBody*       CdlConstConflict_Requires;
typedef const CdlConflict_DataBody*           CdlConstConflict_Data;

typedef const CdlTransactionBody*             CdlConstTransaction;

//}}}
//{{{  Miscellaneous types etc.                         

// ----------------------------------------------------------------------------
// This section is used for data types, function prototypes, etc. which could
// not be defined until after the main CDL classes and handles.

// This typedef is used for error and warning reporting functions.
// Typically such a function pointer will be passed when the library
// is asked to perform any non-trivial parsing operation, e.g. loading
// a package.
//
// If the error is fatal then this callback function should raise
// a CdlParseException.
typedef void (*CdlDiagnosticFnPtr)(std::string);

// ----------------------------------------------------------------------------
// This function is used for update handler. Whenever there is a change
// to CDL entity (it has just been loaded, or its value has changed, or
// whatever) this can affect other CDL entities that reference it.
// All such references occur via properties, and there should be
// update handlers associated with those properties.
//
// Update handlers are also invoked for initialization and finalization
// operations, i.e. when the source object itself has just been loaded
// or is in the process of being unloaded.
//
// The arguments to an update handler are:
// 1) the transaction in which the operation takes place
// 2) the source object containing the reference
// 3) the source property containing the reference
// 4) the destination object. This may be 0 for some update
//    operations.
// 5) an indication of the change that has happened. This should
//    be a CdlUpdate value.
typedef void (*CdlUpdateHandler)(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);

// ----------------------------------------------------------------------------
// This function is also used for transactions. Typically during a
// transaction there will be one or more invocations of the inference engine,
// with callbacks in between to allow one or more of the recommended
// changes to be undone.
typedef CdlInferenceCallbackResult (*CdlInferenceCallback)(CdlTransaction);

// ----------------------------------------------------------------------------
// The TCL API and C++ do not always mesh cleanly, for example a lot
// happens in terms of ClientData which is a void* pointer. To avoid
// too many casts all over the place libcdl provides a CdlInterpreter
// class and the following alternative to Tcl_CmdProc*. A single
// function will be used for the TCL command: its ClientData will be
// the CdlInterpreterCommand, and the CdlInterpreter is accessible via
// AssocData. This does result in some overheads, but none of these
// should be in performance-critical code.
typedef int (*CdlInterpreterCommand)(CdlInterpreter, int, const char*[]);

// ----------------------------------------------------------------------------
// In the libcdl world it is often convenient to swap whole sets of
// commands in and out. For example when executing the body of a
// cdl_component it is desirable to swap in commands for all the
// properties that make sense in a component and swap out all the
// commands that made sense in a higher level. It is assumed that none
// of the commands being swapped in or out are built-ins. Achieving
// this involves a vector of this simple utility structure.
class CdlInterpreterCommandEntry {
  public:
    std::string                 name;
    CdlInterpreterCommand       command;

    CdlInterpreterCommandEntry() : name(""), command(0) {}
    CdlInterpreterCommandEntry(const char *name_arg, CdlInterpreterCommand command_arg)
        : name(name_arg), command(command_arg)
    {
    }
    CdlInterpreterCommandEntry(std::string name_arg, CdlInterpreterCommand command_arg)
        : name(name_arg), command(command_arg)
    {
    }
    ~CdlInterpreterCommandEntry()
    {
        name = "";
        command = 0;
    }
};

// ----------------------------------------------------------------------------
// Persistence support.
// Some applications want to be able to store additional information
// in savefiles, and essentially this involves extra commands that
// get executed when the savefile is executed. It is possible that
// the application reading back the savefile does not understand
// the same set of commands as the application that wrote back the
// data, so the library tries hard not to lose data.
//
// The CdlSaveCallback function typedef is used when installing
// an application-specific savefile command. The first argument
// indicates the node for which the callback is being invoked:
// this may be the entire toplevel, or just an option, or whatever.
//
// The CdlSavefileCommand structure keeps track of the command,
// the save callback if any (non-zero only for application-specific
// data, zero implies that the command is handled by the lirary).
// The load command is invoked when reading in a savefile and the
// appropriate command is executed: unrecognised commands will be
// processed by CdlToplevelBody::savefile_handle_unknown().

typedef void (*CdlSaveCallback)(CdlNode, CdlInterpreter, Tcl_Channel, int);

struct CdlSavefileCommand {
    std::string           name;
    CdlSaveCallback       save_callback;
    CdlInterpreterCommand load_command;
};

// ----------------------------------------------------------------------------
// Widget hint.
// This structure provides widget hint information for a CdlValuable.
// There are separate hints for the bool and data parts, and possibly
// some additional data such as a string identifying the set of
// items in a radio button.
struct CdlWidgetHint {
    CdlBoolWidget       bool_widget;
    CdlValueWidget      value_widget;
    std::string         radio_button_interface;
};

//}}}
//{{{  Memory leak detection                            

// ----------------------------------------------------------------------------
// Provide some macros that are useful for detecting memory leaks. Basically
// there is a static counter for every class, which gets incremented by the
// constructor(s) and decremented by the destructor. Memory leak detection
// is currently enabled if tracing is enabled. It would be possible to use
// another configure-time option, but the overheads of tracing are likely
// to dwarf the overheads of memory leak detection.
//
// For now the memleak counters are always present, even in non-debug
// versions. The overhead is sufficiently small that it can be
// ignored.There is control over whether or not the counters get
// updated in the constructor or destructor. Otherwise there would be problems
// with whether or not there should be a semicolon at the end of the
// CYGDBG_DECLARE_MEMLEAK_COUNTER() macro definition.

#define CYGDBG_DECLARE_MEMLEAK_COUNTER()        static int memleak_counter
#define CYGDBG_DEFINE_MEMLEAK_COUNTER(class)    int class::memleak_counter = 0
#define CYGDBG_GET_MEMLEAK_COUNTER(class)       class::memleak_counter

#ifdef CYGDBG_USE_TRACING

#define CYGDBG_MEMLEAK_CONSTRUCTOR()            this->memleak_counter++;
#define CYGDBG_MEMLEAK_DESTRUCTOR()             this->memleak_counter--;
#define CYGDBG_MEMLEAK_CHECKTHIS()              if (this->memleak_counter < 0) { return false; }
    
#else

#define CYGDBG_MEMLEAK_CONSTRUCTOR()
#define CYGDBG_MEMLEAK_DESTRUCTOR()
#define CYGDBG_MEMLEAK_CHECKTHIS()

#endif

//}}}

//{{{  Cdl class                                        

// ---------------------------------------------------------------------------
// The sole purpose of this class is to provide some utility functions with
// reasonable namespace protection, without requiring that the compiler
// implements namespaces.

class Cdl {
    
  public:

    static bool         is_valid_value_flavor(CdlValueFlavor);
    static bool         is_valid_value_source(CdlValueSource);
    
    static bool         is_valid_cdl_name(const std::string&);
    static bool         is_valid_c_preprocessor_symbol(const std::string&);
    
    static bool         string_to_integer(std::string, cdl_int&);
    static bool         string_to_double(std::string, double&);
    static bool         string_to_bool(std::string, bool&);
    static void         integer_to_string(cdl_int, std::string&, CdlValueFormat = CdlValueFormat_Default);
    static std::string  integer_to_string(cdl_int, CdlValueFormat = CdlValueFormat_Default);
    static void         double_to_string(double, std::string&, CdlValueFormat = CdlValueFormat_Default);
    static std::string  double_to_string(double, CdlValueFormat = CdlValueFormat_Default);
    static void         bool_to_string(bool, std::string&);
    static std::string  bool_to_string(bool);
    static void         integer_to_double(cdl_int, double&);
    static double       integer_to_double(cdl_int);
    static bool         double_to_integer(double, cdl_int&);
    
    static bool         string_to_flavor(std::string, CdlValueFlavor&);
    static bool         flavor_to_string(CdlValueFlavor, std::string&);
    static bool         string_to_source(std::string, CdlValueSource&);
    static bool         source_to_string(CdlValueSource, std::string&);
                                         
    static std::string  get_library_version();
    static void         set_interactive(bool = true);
    static bool         is_interactive();

    static bool         truth() { return true; }
    static bool         falsehood() { return false; }
    
    // return values are -1,0,1 just like strcmp(). The most recent
    // version is the smallest.
    static int          compare_versions(std::string, std::string);

    // Also provide an STL-friendly comparison class
    class version_cmp {
      public:
        bool operator()(const std::string& v1, const std::string& v2) const {
            return Cdl::compare_versions(v1,v2) < 0;
        }
    };

    // Split a version string into major, minor and release numbers.
    static void         split_version_string(const std::string&, std::string& /* major */,
                                             std::string& /* minor */, std::string& /* release */);
    
    // It is occasionally useful to take a full CDL name such as CYGPKG_KERNEL
    // and turn it into a short form, i.e. kernel.
    static std::string  get_short_form(const std::string&);
    
  private:
    static bool         interactive;
};

//}}}
//{{{  CdlInterpreter class                             

// ----------------------------------------------------------------------------
// libcdl requires access to a Tcl interpreter. For now the standard
// interpreter is used. In the long run it may be better to use a
// custom parser in places, if only to improve the diagnostics messages
// that users see.
//
// Consider the case of software CDL (other CDL variants will have
// similar requirements). A Tcl interpreter is needed to read in the
// data for a given package. It will also be needed at various stages
// when the data is being manipulated, e.g. to display a custom dialog
// or to execute e.g. a check_proc or a define_proc. Each package
// should run in its own safe interpreter with limited capabilities:
// file I/O is limited to read-only, but read-write in the build and
// install trees; network I/O is out of the question, at least until
// appropriate security support is added to the CDL language itself.
// However the interpreter should be extended with additional commands
// like cdl_get and cdl_set to access the configuration data.
//
// For security and robustness reasons it is desirable to have
// separate interpreters for the various packages. This leads to the
// concept of a master interpreter for the entire configuration, and a
// group of slave interpreters, one per package. In this model it
// is convenient to have the configuration and package entities
// associated directly with the interpreter. Note that a single
// application may have several configurations loaded in memory,
// so there may be several master interpreters.
//
// Some applications will want to support the graphical side of CDL,
// i.e. custom dialogs and wizards. This means linking in Tk, not to
// mention X11 (or the Windows equivalents), and making some/all of
// the Tk commands available to the safe interpreter. Arguably
// commands like toplevel should always be disabled. Not all clients
// of libcdl will want the overheads of linking with Tk and X, so this
// has to be made optional.
//
// The approach taken is as follows:
//
// 1) there is a class CdlInterpreter which provides access to Tcl
//    interpreters. Amongst other things it takes care of converting
//    between C and C++ strings.
//
// 2) every toplevel needs its own CdlInterpreter. The application
//    code should supply this interpreter itself when the toplevel
//    is instantiated, allowing it to decide whether or not Tk should
//    be available.
//
// 3) each loadable gets its own safe slave interpreter, derived from
//    the toplevel's interpreter.
//    NOTE: initially the slave interpreters are not actually safe. It
//    is not clear in the long term to what extent per-loadable
//    interpreters need to be sandboxes, there are issues such as
//    doing the equivalent of autoconf tests.

// Tcl 8.4 involved various incompatible API changes related to
// const vs. non-const data. #define'ing USE_NON_CONST or
// USE_COMPAT_CONST avoids some of the problems, but does not
// help much for C++.
#if (TCL_MAJOR_VERSION > 8) || ((TCL_MAJOR_VERSION == 8) && (TCL_MINOR_VERSION >= 4))
# define CDL_TCL_CONST_CAST(type,var) (var)
#else
# define CDL_TCL_CONST_CAST(type,var) const_cast<type>(var)
#endif

class CdlInterpreterBody
{
    friend class        CdlTest;

  public:

    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
    // This is how a top-level (i.e. per-toplevel) interpreter
    // should get created.
    static CdlInterpreter       make(Tcl_Interp* = 0);

    // Create a slave interpreter for reading in the data in e.g. a
    // cdl_package
    CdlInterpreter create_slave(CdlLoadable, bool /* safe */ = true);
    
    // Make the interpreter safe, a one-way operation.
    void                make_safe();
    
    // The destructor is public.
    virtual ~CdlInterpreterBody();

    // Add or remove commands from an interpreter. This provides
    // a more C++-friendly implementation of Tcl's
    // CreateCommand() and DeleteCommand().
    void add_command(std::string, CdlInterpreterCommand);
    void remove_command(std::string);

    // In the libcdl world it is also convenient to swap whole sets of
    // commands in and out. This is achieved by push and pop operations.
    // push returns the old set (0 at the toplevel). pop restores
    // the old set.
    std::vector<CdlInterpreterCommandEntry>* push_commands(std::vector<CdlInterpreterCommandEntry>&);
    void pop_commands(std::vector<CdlInterpreterCommandEntry>*);
    std::vector<CdlInterpreterCommandEntry>* get_pushed_commands() const;
    
    // Similarly, allow variables to be set, unset and queried
    void        set_variable(std::string, std::string);
    void        unset_variable(std::string);
    std::string get_variable(std::string);
    
    // FIXME: add support for variable traces. These are needed
    // for cdl_value and similar utilities.
    
    // Provide hooks into the AssocData() facilities associated with
    // Tcl interpreters. This makes it possible to store arbitrary
    // data with an interpreter, e.g. to keep track of current state.
    void       set_assoc_data(const char*, ClientData, Tcl_InterpDeleteProc* =0);
    void       delete_assoc_data(const char*);
    ClientData get_assoc_data(const char*);

    // Evaluate a string as Tcl code. The return value comes from Tcl, e.g.
    // TCL_OK or TCL_ERROR. There are variants depending on whether or not
    // the result string is of interest.
    int eval(std::string);
    int eval(std::string, std::string&);

    // Ditto for any Tcl code that comes from CDL files
    int eval_cdl_code(const cdl_tcl_code);
    int eval_cdl_code(const cdl_tcl_code, std::string&);

    // And support for evaluating an entire file
    int eval_file(std::string);
    int eval_file(std::string, std::string&);
    
    // For use by commands implemented in C++, a way of setting the result
    void set_result(std::string);

    // And a utility to get the result as well.
    std::string get_result();

    // Was the result set by the Tcl interpreter or by libcdl?
    bool result_set_by_cdl();
    
    // A utility to quote data that is going to end up in a TCL script.
    static std::string quote(std::string);

    // Turn some multiline data into a comment.
    static std::string multiline_comment(const std::string&, int, int = 0);

    // Add some data to a comment, allowing for newlines if necessary
    static std::string extend_comment(const std::string&, int, int = 0);
    
    // Write some data to a savefile, throwing an exception on error
    void write_data(Tcl_Channel, std::string);

    // File-related utilities.
    void locate_subdirs(std::string, std::vector<std::string>&);
    void locate_all_subdirs(std::string, std::vector<std::string>&);
    void locate_files(std::string, std::vector<std::string>&);
    void locate_all_files(std::string, std::vector<std::string>&);
    bool is_directory(std::string);
    bool is_file(std::string);

    // When parsing a CDL script it is convenient to keep track of
    // a number of items:
    //
    // 1) the toplevel, e.g. the entire configuration
    // 2) the loadable, e.g. the current package
    // 3) the parent of whatever is being processed at the moment
    // 4) the entity, i.e. the thingamajig that is being processed.
    // 5) the current file
    // 6) an error reporting function
    //
    // This gives the various commands embedded in the Tcl interpreter
    // enough information to do their job. Additional information can
    // be provided via assoc_data()
    //
    // There should be only one call to set_toplevel(), for the
    // master interpreter. All slaves inherit this, and the toplevel
    // cannot be changed again.
    //
    // The loadable field is filled in via make_slave()
    //
    // For some members push and pop functions are more appropriate
    // than set.
    CdlToplevel         get_toplevel() const;
    CdlLoadable         get_loadable() const;
    CdlContainer        get_container() const;
    CdlNode             get_node() const;
    std::string         get_context() const;
    CdlDiagnosticFnPtr  get_error_fn_ptr() const;
    CdlDiagnosticFnPtr  get_warning_fn_ptr() const;
    CdlTransaction      get_transaction() const;
    void                set_toplevel(CdlToplevel);
    void                set_transaction(CdlTransaction);
    CdlContainer        push_container(CdlContainer);
    void                pop_container(CdlContainer);
    CdlNode             push_node(CdlNode);
    void                pop_node(CdlNode);
    std::string         push_context(std::string);
    void                pop_context(std::string);
    CdlDiagnosticFnPtr  push_error_fn_ptr(CdlDiagnosticFnPtr);
    void                pop_error_fn_ptr(CdlDiagnosticFnPtr);
    CdlDiagnosticFnPtr  push_warning_fn_ptr(CdlDiagnosticFnPtr);
    void                pop_warning_fn_ptr(CdlDiagnosticFnPtr);

    // Provide utility classes for common push/pop combinations. The
    // push happens during the constructor, the pop during the
    // destructor. This can simplify some code, especially when
    // exceptions may get thrown.
    class DiagSupport {
      public:
        DiagSupport(CdlInterpreter interp_arg, CdlDiagnosticFnPtr error_fn_arg, CdlDiagnosticFnPtr warn_fn_arg) {
            interp         = interp_arg;
            saved_error_fn = interp->push_error_fn_ptr(error_fn_arg);
            saved_warn_fn  = interp->push_warning_fn_ptr(warn_fn_arg);
        }
        ~DiagSupport() {
            interp->pop_error_fn_ptr(saved_error_fn);
            interp->pop_warning_fn_ptr(saved_warn_fn);
        }
    private:
        DiagSupport();

        CdlInterpreter     interp;
        CdlDiagnosticFnPtr saved_error_fn;
        CdlDiagnosticFnPtr saved_warn_fn;
    };
    class ContextSupport {
      public:
        ContextSupport(CdlInterpreter interp_arg, std::string context) {
            interp = interp_arg;
            saved_context = interp->push_context(context);
        }
        ~ContextSupport() {
            interp->pop_context(saved_context);
        }
      private:
        ContextSupport();
        CdlInterpreter interp;
        std::string    saved_context;
    };
    class ContainerSupport {
      public:
        ContainerSupport(CdlInterpreter interp_arg, CdlContainer container) {
            interp = interp_arg;
            saved_container = interp->push_container(container);
        }
        ~ContainerSupport() {
            interp->pop_container(saved_container);
        }
      private:
        ContainerSupport();
        CdlInterpreter interp;
        CdlContainer   saved_container;
    };
    class NodeSupport {
      public:
        NodeSupport(CdlInterpreter interp_arg, CdlNode node) {
            interp = interp_arg;
            saved_node = interp->push_node(node);
        }
        ~NodeSupport() {
            interp->pop_node(saved_node);
        }
      private:
        NodeSupport();
        CdlInterpreter interp;
        CdlNode        saved_node;
    };
    class CommandSupport {
      public:
        CommandSupport(CdlInterpreter interp_arg, std::vector<CdlInterpreterCommandEntry>& commands) {
            interp = interp_arg;
            saved_commands = interp->push_commands(commands);
        }
        CommandSupport(CdlInterpreter interp_arg, CdlInterpreterCommandEntry* commands) {
            unsigned int i;
            for (i = 0; 0 != commands[i].command; i++) {
                new_commands.push_back(commands[i]);
            }
            interp = interp_arg;
            saved_commands = interp->push_commands(new_commands);
        }
        ~CommandSupport() {
            interp->pop_commands(saved_commands);
        }

      private:
        CommandSupport();
        CdlInterpreter interp;
        std::vector<CdlInterpreterCommandEntry>* saved_commands;
        std::vector<CdlInterpreterCommandEntry> new_commands;
    };

    // Similar utility classes for variables and assoc data.
    class VariableSupport {
      public:
        VariableSupport(CdlInterpreter interp_arg, std::string varname_arg, std::string data) {
            interp  = interp_arg;
            varname = varname_arg;
            interp->set_variable(varname, data);
        }
        ~VariableSupport() {
            interp->unset_variable(varname);
        }
      private:
        VariableSupport();
        CdlInterpreter interp;
        std::string    varname;
    };
    class AssocSupport {
      public:
        AssocSupport(CdlInterpreter interp_arg, const char* name_arg, ClientData data, Tcl_InterpDeleteProc* del_proc = 0) {
            interp = interp_arg;
            name   = name_arg;
            interp->set_assoc_data(name, data, del_proc);
        }
        ~AssocSupport() {
            interp->delete_assoc_data(name);
        }
      private:
        AssocSupport();
        CdlInterpreter interp;
        const char*    name;
    };
    
    // Some command implementations may want to access other Tcl library
    // routines such as Tcl_SplitList(). This requires convenient access
    // to the underlying Tcl interpreter.
    Tcl_Interp*         get_tcl_interpreter() const;
    
    // For use by the assertion macros.
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;

  private:
    // This is the Tcl command proc that gets registered for all
    // CdlInterpreterCommand instances.
    static int          tcl_command_proc(ClientData, Tcl_Interp*, int, const char*[]);

    // This key is used to access the CdlInterpreter assoc data.
    static char*        cdlinterpreter_assoc_data_key;
    
    // Do not allow static instances of a Cdl interpreter. There are too
    // many possible failure conditions. Cdl interpreters can only be
    // created dynamically via make(), which will invoke this.
    CdlInterpreterBody(Tcl_Interp*);

    // Default constructor, copy constructor and assignment are illegal
    CdlInterpreterBody();
    CdlInterpreterBody(const CdlInterpreterBody&);
    CdlInterpreterBody& operator=(const CdlInterpreterBody&);

    
    Tcl_Interp*                 tcl_interp;     // The underlying Tcl interpreter
    bool                        owns_interp;    // Was the Tcl interpreter created by the library?
    std::vector<CdlInterpreter> slaves;         // All slave interpreters
    CdlInterpreter              parent;         // Or else the parent
    CdlToplevel                 toplevel;       // Data that gets used during the parsing process
    CdlTransaction              transaction;
    CdlLoadable                 loadable;
    CdlContainer                container;
    CdlNode                     node;
    std::string                 context;
    CdlDiagnosticFnPtr          error_fn_ptr;
    CdlDiagnosticFnPtr          warning_fn_ptr;
    bool                        cdl_result;
    
    std::vector<CdlInterpreterCommandEntry>* current_commands; // for push() and pop()
    
    enum {
        CdlInterpreterBody_Invalid = 0,
        CdlInterpreterBody_Magic   = 0x0be67689
    } cdlinterpreterbody_cookie;
};

//}}}
//{{{  CdlReference/Referrer classes                    

// ---------------------------------------------------------------------------
// CDL objects are organised primarily in a tree hierarchy. For
// example a package contains components, components contain options,
// and so on. The tree hierarchy tends to change rather infrequently,
// so it makes sense to have a quick way of navigating between
// entities without continuously having to do hash-table lookups. In
// addition it is very desirable to make the connectivity
// bidirectional: if a "requires" property in option A references
// option B then it would be useful to have a link back to A from B;
// that way, if the value of B changes it is a lot easier to keep
// things up to date.
//
// Terminology: the entity which contains the reference, e.g. a
// "requires" property, is the source. The relevant property is the
// "source property". The entity pointed at is the destination.
//
// Unfortunately there may be connections between CDL entities outside
// the tree hierarchy. In particular any property can contain one or
// more references to packages, components, options, wizards, or
// whatever. Often these references will be to options etc. within the
// same package, but some references will go to other packages. There
// may even be references to other configurations: for example a board
// may contain both an ordinary processor and a DSP; these two need
// their own configurations; however a package running on the DSP may
// need to interact with a package running on the processor, and vice
// versa.
//
// Also, a reference may occur inside an object that is not in the
// hierarchy. For example CDL expressions may get evaluated inside Tcl
// code rather than as part of a property. Such expressions may still
// contain references to entities in the current configuration.
//
// References may not be resolved. When reading in a CDL script there
// may be forward references. A reference may involve another package
// that has not yet been loaded, which is a conflict.
//
// Using simple pointers to store these connections is a bad idea. It
// makes it a lot harder to figure out what is connected to what, and
// it introduces horrible consistency problems when packages get
// loaded and unloaded. Instead libCDL provides a CdlReference class.
// Whenever a CdlProperty contains a reference to some other CDL
// entity there should be a CdlReference object corresponding to this.
// The reverse direction is handled via a CdlReferrer object.
//
// A CdlReference object can be either bound or unbound. By default it
// is unbound, containing only a string. It can then be bound via a
// member function, examined, and unbound again as required. Creating
// a binding automatically creates a CdlReferrer entry in the target
// object, thus avoiding any risk of inconsistencies.
//
// The CdlReference class should not be used outside the hierarchy,
// since every bound reference must have a referrer object pointing
// back, and this link back can only be valid within the hierarchy.
// Temporary CdlReference objects are useful during the construction
// of properties.
//
// It is possible that a given property (e.g. a complicated "requires"
// expression) has multiple references to another entity. Each of
// these involves a separate CdlReference/CdlReferrer pair.

// ----------------------------------------------------------------------------
// The actual CdlReference class.

class CdlReference {

    friend class        CdlTest;

    // CdlReferrer must be a friend so that when a package gets unloaded
    // it can clean up all references to it.
    friend class        CdlReferrer;
    
  public:

    // The default constructor should not normally be used, instead
    // a string should be supplied. However there are vectors of
    // reference objects...
    CdlReference();
    
    // The main constructor supplies the name of the referenced
    // entity. The resulting object will be unbound.
    CdlReference(const std::string);

    // The copy constructor is legal for unbound objects only.
    CdlReference(const CdlReference&);

    // The assignment operator is needed for STL operations.
    // Again it only makes sense of unbound objects.
    CdlReference& operator=(const CdlReference&);
    
    // The destructor is only valid for unbound objects. All references
    // should be unbound before an entity can be destroyed.
    ~CdlReference();
    
    // Access the various fields.
    void               set_destination_name(const std::string);
    const std::string& get_destination_name() const;
    CdlNode            get_destination() const;

    // Binding a reference. Obviously this can only be used when the
    // reference is still unbound. When doing the binding it is
    // necessary to know:
    //   (1) the object containing the reference.
    //   (2) the specific property that contains the reference.
    //   (3) the object being referred to.
    // Binding a reference results in a new referrer entry in the
    // destination.
    void bind(CdlNode, CdlProperty, CdlNode);

    // Unbinding a reference. Typically this only happens when the
    // destination is unloaded. The arguments provide the source and
    // the source property.
    void unbind(CdlNode, CdlProperty);

    // This is used by the ASSERT_CLASS() and ASSERT_THIS() macros.
    bool check_this(cyg_assert_class_zeal cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:

  private:
    
    // The data fields. The name is usually filled in by the
    // constructor. The destination defaults to zero for an unbound
    // object and gets filled in by the bind() operation.
    std::string dest_name;
    CdlNode     dest;

    enum {
        CdlReference_Invalid = 0,
        CdlReference_Magic   = 0x3f908608
    } cdlreference_cookie;
};

// ----------------------------------------------------------------------------
// The CdlNode class (and hence just about everything) contains a
// vector of CdlReferrer objects. This keeps track of all entities
// that refer to this one, so if the value associated with this
// changes it is possible to work out the impact of this on all
// entities that rely on this value.
//
// Arguably this should work in terms of CdlValuable objects rather
// than CdlNode objects. However it is convenient to use references
// for the connection between e.g. an option and a dialog, where
// there is no value involved. The reverse connection is of little
// use in this circumstance.
//
// CdlReferrer objects are rarely accessed directly. Instead they will
// be filled in during a CdlReference::bind() operation and erased
// during a CdlReference::unbind() operation. The only operations that
// should be public allow access to the contained data.

class CdlReferrer {

    friend class        CdlTest;

    // CdlReference::bind() and unbind() have direct access to the
    // members, since these two functions are really responsible for
    // creating and destroying referrer objects.
    friend class        CdlReference;
    
  public:

    // The default constructor, copy constructor and assignment
    // operator are all public to avoid problems with having vectors
    // of referrer objects. Similarly the destructor is public.
    // In practice updates actually happen as a consequence of
    // CdlReference::bind() and CdlReference::unbind().
    CdlReferrer();
    CdlReferrer(const CdlReferrer&);
    CdlReferrer& operator=(const CdlReferrer&);
    ~CdlReferrer();

    CdlNode     get_source() const;
    CdlProperty get_source_property() const;
    void        update(CdlTransaction, CdlNode, CdlUpdate);
    bool        check_this(cyg_assert_class_zeal=cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  private:

    CdlNode     source;
    CdlProperty source_property;
    
    enum {
        CdlReferrer_Invalid = 0,
        CdlReferrer_Magic   = 0x70e1fc37
    } cdlreferrer_cookie;
};

//}}}
//{{{  Value and Expression  classes                    

//{{{  CdlEvalContext                   

// ----------------------------------------------------------------------------
// Expression evaluation always happens within a certain context.
// This may involve a transaction. Usually it involves a node and
// a property within that node, although it is possible to evaluate
// expressions from inside Tcl code.
//
// To avoid passing too many arguments around the various
// evaluation-related routines, a utility class is provided.

class CdlEvalContext {
    
    friend class CdlTest;
    
  public:

    CdlTransaction      transaction;
    CdlNode             node;
    CdlProperty         property;
    CdlToplevel         toplevel;

    CdlEvalContext(CdlTransaction, CdlNode = 0, CdlProperty = 0, CdlToplevel = 0);
    ~CdlEvalContext();

    // Given a reference inside an expression, try to resolve this to either
    // a node or, more specifically, a valuable.
    CdlNode             resolve_reference(CdlExpression, int);
    CdlValuable         resolve_valuable_reference(CdlExpression, int);
    
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:

  private:
    // Illegal operation, the three fields must always be supplied,
    // although they may be zero.
    CdlEvalContext();

    enum {
        CdlEvalContext_Invalid  = 0,
        CdlEvalContext_Magic    = 0x03434be9
    } cdlevalcontext_cookie;
    
};

//}}}
//{{{  CdlSimpleValue                   

// ----------------------------------------------------------------------------
// Expression evaluation happens in terms of CdlSimpleValue objects.
// In CDL all values are strings, but for the purposes of arithmetic
// these strings sometimes have to be interpreted as integers or as
// double precision numbers. Sometimes there is a choice, for example
// the equality operator == can mean numerical or string comparison.
// The basic rules that get applied are:
//
//    1) if the current value has an integer representation then
//       use this by preference. This means that an expression
//       of the form (CYGNUM_XXX != 0x100) will do a integer
//       comparison if possible.
//
//    2) otherwise if the current value can be interpreted as a
//       double precision number, use that representation.
//       All integers can be interpreted as doubles (at the risk
//       of some loss of precision), so the representation as
//       a double should only be used if the integer representation
//       is inappropriate.
//
//    3) otherwise interpret the value as a string.
//
// The default value is 0.

class CdlSimpleValue {
    
    friend class CdlTest;

  public:

    CdlSimpleValue();
    CdlSimpleValue(std::string);
    CdlSimpleValue(cdl_int);
    CdlSimpleValue(double);
    CdlSimpleValue(const CdlSimpleValue&);
    CdlSimpleValue(bool);
    ~CdlSimpleValue();

    CdlSimpleValue&     operator=(const CdlSimpleValue&);
    CdlSimpleValue&     operator=(std::string);
    CdlSimpleValue&     operator=(cdl_int);
    CdlSimpleValue&     operator=(double);
    
    CdlSimpleValue&     operator=(bool);
    
    bool                operator==(const CdlSimpleValue&) const;
    bool                operator!=(const CdlSimpleValue&) const;
    bool                operator==(std::string arg) const
    {
        CdlSimpleValue val(arg);
        return *this == val;
    }
    bool                operator==(cdl_int arg) const
    {
        CdlSimpleValue val(arg);
        return *this == val;
    }
    bool                operator==(double arg) const
    {
        CdlSimpleValue val(arg);
        return *this == val;
    }
    bool                operator!=(std::string arg) const
    {
        CdlSimpleValue val(arg);
        return *this != val;
    }
    bool                operator!=(cdl_int arg) const
    {
        CdlSimpleValue val(arg);
        return *this != val;
    }
    bool                operator!=(double arg) const
    {
        CdlSimpleValue val(arg);
        return *this != val;
    }
    
    void                set_value(std::string, CdlValueFormat = CdlValueFormat_Default);
    std::string         get_value() const;
    
    bool                has_integer_value() const;
    void                set_integer_value(cdl_int, CdlValueFormat = CdlValueFormat_Default);
    cdl_int             get_integer_value() const;
    
    bool                has_double_value() const;
    void                set_double_value(double, CdlValueFormat = CdlValueFormat_Default);
    double              get_double_value() const;

    CdlValueFormat      get_value_format() const;
    void                set_value_format(CdlValueFormat);
    void                set_value_format(CdlSimpleValue&);
    void                set_value_format(CdlSimpleValue&, CdlSimpleValue&);

    static void         eval_valuable(CdlEvalContext&, CdlValuable, CdlSimpleValue&);
    
    // For expression evaluation, it is often convenient to get hold
    // of a boolean as well. This may indicate a non-empty string
    // or a non-zero value.
    bool                get_bool_value() const;
    
    // This class is too simple to warrant even a cookie validation.
    bool check_this(cyg_assert_class_zeal zeal = cyg_quick) const {
        return true;
    }
    
  protected:

  private:
    enum {
        int_valid       = 0x01,
        double_valid    = 0x02,
        string_valid    = 0x04,
        int_invalid     = 0x08,
        double_invalid  = 0x10
    };
    mutable int         valid_flags;
    mutable std::string value;
    mutable cdl_int     int_value;
    mutable double      double_value;
    CdlValueFormat      format;
};

//}}}
//{{{  CdlListValue                     

// ----------------------------------------------------------------------------
// Evaluating a list expression results in a set of possible values, but
// unlike the original list expression these values are now constant and
// can have no dependencies on CDL entities. As with list expressions the
// main operation on a list value is to detect membership, but using
// list values allows multiple potential members to be tested without
// repeated expression evaluation. The main use of list values is implicit
// in libcdl, each list expression contains a mutable cached list value.
//
// A list value contains five sets of data:
//
// 1) separate vectors of strings, integers, and floating point constants.
//    Having separate vectors of integers and floating points avoids
//    problems when numbers can be represented in different formats.
// 2) a vector of cdl_int pairs for ranges of integer data
// 3) a vector of double pairs for ranges of floating point data
//
// Any of these vectors may be empty, but at least one of the vectors should
// contain useful data. Possibly there should also be tables for cdl_int and
// double to avoid unnecessary string conversions.

class CdlListValue {
    
    friend class        CdlTest;

    // A list value will only be filled in when a list expression is evaluated.
    // The members cannot be updated by other means.
    friend class        CdlListExpressionBody;
    
  public:

    CdlListValue();
    ~CdlListValue();
    CdlListValue(const CdlListValue&);
    CdlListValue& operator=(const CdlListValue&);

    bool        is_member(CdlSimpleValue&) const;
    bool        is_member(std::string, bool = true) const;
    bool        is_member(cdl_int, bool = true) const;
    bool        is_member(double, bool = true) const;

    // These provide access to the raw data, for example if it is
    // necessary to suggest a legal value to the user.
    const std::vector<CdlSimpleValue>&                get_table() const;
    const std::vector<std::pair<cdl_int, cdl_int> >&  get_integer_ranges() const;
    const std::vector<std::pair<double, double> >&    get_double_ranges() const;

    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  private:
    std::vector<CdlSimpleValue>                 table;
    std::vector<std::pair<cdl_int, cdl_int> >   integer_ranges;
    std::vector<std::pair<double, double> >     double_ranges;

    enum {
        CdlListValue_Invalid  = 0,
        CdlListValue_Magic    = 0x2183a943
    } cdllistvalue_cookie;
};

//}}}
//{{{  CdlValue                         

// ----------------------------------------------------------------------------
// Values in CDL are non-trivial compared with some other languages.
// Even though CDL is not a fully-typed language, it does still have
// four different flavors to consider. There is also the problem that
// an entity may have up to four different values which should be
// stored (default, inferred, wizard, user), with the ability to
// switch between them. The CdlValue class provides support for this.
//
// CdlValue objects are not normally updated explicitly. Instead
// higher level code deals with CdlValuable objects, which inherit
// privately from CdlValue. Modifications to CdlValuables happen in
// the context of a transaction.
//
// The first concept to take into account is the flavor. There
// are four flavors, None, Bool, BoolData and Data. The member
// function get_flavor() can be used to obtain the current flavor.
//
//     CdlValueFlavor CdlValue::get_flavor() const;
//
// Values may be enabled or disabled. Values of flavor None
// and Data are always enabled. Values of flavor Bool or BoolData
// may or may not be enabled, by default they are disabled.
//
//     bool CdlValue::is_enabled(...) const;
//
// (The optional argument to is_enabled() is discussed later).
//
// Values of flavor BoolData and Data also have a string value,
// which can be interpreted as an integer or a double under
// the right circumstances.
//
//     std::string CdlValue::get_value(...) const;
//     bool        CdlValue::has_integer_value(...) const;
//     bool        CdlValue::has_double_value(...) const;
//     cdl_int     CdlValue::get_integer_value(...) const;
//     double      CdlValue::get_double_value(...) const;
//
// This is equivalent to a CdlSimpleValue object, and in fact
// that is the internal representation. It is possible to
// get hold of the CdlSimpleValue object directly:
//
//     CdlSimpleValue CdlValue::get_simple_value(...) const;
//
// The get_integer_value() and get_double_value() members should
// only be used if you are confident that the current value has
// an integer or double representation. Otherwise the result is
// undefined.
//
// The optional argument to these member functions represents
// the source. A value can be set from four different sources:
// the default value (usually either 0 or the result of
// evaluating a default_value property); an inferred value,
// determined by the inference engine; a wizard value, i.e.
// what a CDL wizard believes the correct value to be based
// on user input; and a user value, something explicitly
// set by the end user. These have different priorities:
// the inference engine can override default values but not
// user values. A CdlValue object keeps track of the current
// source.
//
//    CdlValueSource CdlValue::get_source() const;
//
// If no argument is given to e.g. is_enabled() then the
// current source is used. Otherwise it is possible to find
// out whether or not the entity is enabled for each of the
// sources.
//
// The default source is always defined, the others may or
// may not be. It is possible to find out for each source
// whether or not a value has been set.
//
//   bool CdlValue::has_source(CdlValueSource) const;
//
//
// Updating values normally happens in the CdlValuable class,
// but the member functions are the same. There is a member
// function to change the flavor:
//
//   void CdlValue::set_flavor(CdlValueFlavor);
//
// However this member function is intended only for use by the
// library itself. An entity's flavor is normally defined by CDL data,
// and should not be updated explicitly by application code.
//
// There are two member functions to manipulate the value source:
//
//     void CdlValue::set_source(CdlValueSource);
//     void CdlValue::invalidate_source(CdlValueSource);
//
// The first function can be used if e.g. the user wants to
// change his or her mind and go back to the default value
// rather than a user value. The user value is not forgotten
// and can be reinstated. 
//
// invalidate_source() can be used to completely cancel a
// value source. If that source happens to be the current one
// then the current source will be adjusted appropriately.
// It is illegal to attempt to invalidate the default source.
//
// For values with flavor Bool and BoolData, there are three
// member functions that can be used to control the enabled
// status:
//
//   void CdlValue::set_enabled(bool, CdlValueSource);
//   void CdlValue::enable(CdlValueSource);
//   void CdlValue::disable(CdlValueSource);
//
// Note that when updating a CdlValue object the source should
// be known and must be specified. If the source has a higher
// priority than the current one then it will automatically
// become the new source. On the rare occasion that this is
// not desired, set_source() will have to be used afterwards
// to reset the current source.
//
// For values with flavor BoolData and Data the following
// member functions are available to change the value string:
//
//   void CdlValue::set_value(std::string, CdlValueSource);
//   void CdlValue::set_value(cdl_int, CdlValueSource);
//   void CdlValue::set_value(double, CdlvalueSource);
//   void CdlValue::set_value(CdlSimpleValue&, CdlValueSource);
//
// For values with flavor BoolData is is possible to
// combine updating the enabled flag and the string value:
//
//   void CdlValue::set_enabled_and_value(bool, std::string, CdlValueSource);
//   void CdlValue::set_enabled_and_value(bool, cdl_int, CdlValueSource);
//   void CdlValue::set_enabled_and_value(bool, double, CdlValueSource);
//   void CdlValue::set_enabled_and_value(bool, CdlSimpleValue&, CdlValueSource);
//   void CdlValue::enable_and_set_value(std::string, CdlValueSource);
//   void CdlValue::enable_and_set_value(cdl_int, CdlValueSource);
//   void CdlValue::enable_and_set_value(double, CdlValueSource);
//   void CdlValue::enable_and_set_value(CdlSimpleValue&, CdlValueSource);
//   void CdlValue::disable_and_set_value(std::string, CdlValueSource);
//   void CdlValue::disable_and_set_value(cdl_int, CdlValueSource);
//   void CdlValue::disable_and_set_value(double, CdlValueSource);
//   void CdlValue::disable_and_set_value(CdlSimpleValue&, CdlValueSource);
//
// Obviously many of these functions are just simple inlines.
//
// There is one final member function:
//
//   void CdlValue::set(CdlSimpleValue, CdlValueSource);
//
// This member function is defined to do the right thing,
// whatever the flavor happens to be.

class CdlValue {
    
    friend class CdlTest;

  public:

    CdlValue(CdlValueFlavor = CdlValueFlavor_Bool);
    virtual ~CdlValue();
    CdlValue(const CdlValue&);
    CdlValue&           operator=(const CdlValue&);

    CdlValueFlavor      get_flavor() const;
    CdlValueSource      get_source() const;
    bool                has_source(CdlValueSource) const;

    bool                is_enabled(CdlValueSource = CdlValueSource_Current) const;
    std::string         get_value(CdlValueSource = CdlValueSource_Current) const;
    bool                has_integer_value(CdlValueSource = CdlValueSource_Current) const;
    bool                has_double_value(CdlValueSource = CdlValueSource_Current) const;
    cdl_int             get_integer_value(CdlValueSource = CdlValueSource_Current) const;
    double              get_double_value(CdlValueSource = CdlValueSource_Current) const;
    CdlSimpleValue      get_simple_value(CdlValueSource = CdlValueSource_Current) const;
    
    void set_source(CdlValueSource);
    void invalidate_source(CdlValueSource);
    
    void set_enabled(bool, CdlValueSource);
    void enable(CdlValueSource source)
    {
        set_enabled(true, source);
    }
    void disable(CdlValueSource source)
    {
        set_enabled(false, source);
    }

    void set_value(CdlSimpleValue&, CdlValueSource);
    void set_value(std::string data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_value(val, source);
    }
    void set_integer_value(cdl_int data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_value(val, source);
    }
    void set_double_value(double data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_value(val, source);
    }
    void set_enabled_and_value(bool, CdlSimpleValue&, CdlValueSource);
    void set_enabled_and_value(bool enabled, std::string data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_enabled_and_value(enabled, val, source);
    }
    void set_enabled_and_value(bool enabled, cdl_int data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_enabled_and_value(enabled, val, source);
    }
    void set_enabled_and_value(bool enabled, double data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_enabled_and_value(enabled, val, source);
    }
    void enable_and_set_value(CdlSimpleValue& val, CdlValueSource source)
    {
        set_enabled_and_value(true, val, source);
    }
    void enable_and_set_value(std::string data, CdlValueSource source)
    {
        set_enabled_and_value(true, data, source);
    }
    void enable_and_set_value(cdl_int data, CdlValueSource source)
    {
        set_enabled_and_value(true, data, source);
    }
    void enable_and_set_value(double data, CdlValueSource source)
    {
        set_enabled_and_value(true, data, source);
    }
    void disable_and_set_value(CdlSimpleValue& val, CdlValueSource source)
    {
        set_enabled_and_value(false, val, source);
    }
    void disable_and_set_value(std::string data, CdlValueSource source)
    {
        set_enabled_and_value(false, data, source);
    }
    void disable_and_set_value(cdl_int data, CdlValueSource source)
    {
        set_enabled_and_value(false, data, source);
    }
    void disable_and_set_value(double data, CdlValueSource source)
    {
        set_enabled_and_value(false, data, source);
    }

    void set(CdlSimpleValue&, CdlValueSource);
    
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

    // This should only be used by the library itself.
    void                set_flavor(CdlValueFlavor);
    
  protected:
    
  private:
    
    CdlValueFlavor      flavor;
    CdlValueSource      current_source;

    // FIXME: a static const member should be used for the array
    // sizes, but VC++ does not support that part of the language.
    // FIXME: std::bitset should be used here. Using lots of separate
    // bools here is inefficient.
    bool                source_valid[4];
    bool                enabled[4];
    CdlSimpleValue      values[4];
    
    enum {
        CdlValue_Invalid = 0,
        CdlValue_Magic   = 0x41837960
    } cdlvalue_cookie;
};

//}}}
//{{{  CdlSubexpression                 

// ----------------------------------------------------------------------------
// Expressions come into existence primarily as the result of reading
// in certain properties like default_value in CDL data. It is also
// possible for expressions to be generated and evaluated on the fly
// inside Tcl code, but that is expected to be a comparatively rare
// events. Expression objects always live on the heap, usually only
// in derived classes.
//
// An ordinary expression evaluates to a single value. There are two
// other types of expression in the CDL language, goal expressions and
// list expression. A goal expression is essentially a set of ordinary
// expressions with implicit &&'s between them. A list expression
// is a set of expressions that can be evaluated to a constant vector,
// plus pairs of expressions that constitute ranges. Again goal and
// list expressions only live on the heap.
//
// Both parsing an evaluation involve tokens for the various
// operators. The inference engine, conflict reporting code, and
// other diagnostic code will also need to have ready access to
// this information. Hence it makes a bit more sense to have
// the enum outside the expression class.

enum CdlExprOp {
    CdlExprOp_Invalid           =  0,
    CdlExprOp_EOD               =  1,   // End of data reached
    CdlEXprOp_Command           =  2,   // [tcl code]
    CdlExprOp_Variable          =  3,   // $tcl_variable
    CdlExprOp_StringConstant    =  4,   // "hello"
    CdlExprOp_IntegerConstant   =  5,   // 123
    CdlExprOp_DoubleConstant    =  6,   // 3.1415
    CdlExprOp_Reference         =  7,   // CYGPKG_INFRA
    CdlExprOp_Range             =  8,   // x to y
    CdlExprOp_Negate            =  9,   // -x
    CdlExprOp_Plus              = 10,   // +x
    CdlExprOp_LogicalNot        = 11,   // !x
    CdlExprOp_BitNot            = 12,   // ~x
    CdlExprOp_Indirect          = 13,   // *x
    CdlExprOp_Active            = 14,   // ?x
    CdlExprOp_Function          = 15,   // sin(x)
    CdlExprOp_Multiply          = 16,   // x * y
    CdlExprOp_Divide            = 17,   // x / y
    CdlExprOp_Remainder         = 18,   // x % y
    CdlExprOp_Add               = 19,   // x + y
    CdlExprOp_Subtract          = 20,   // x - y
    CdlExprOp_LeftShift         = 21,   // x << y
    CdlExprOp_RightShift        = 22,   // x >> y
    CdlExprOp_LessThan          = 23,   // x < y
    CdlExprOp_LessEqual         = 24,   // x <= y
    CdlExprOp_GreaterThan       = 25,   // x > y
    CdlExprOp_GreaterEqual      = 26,   // x >= y
    CdlExprOp_Equal             = 27,   // x == y
    CdlExprOp_NotEqual          = 28,   // x != y
    CdlExprOp_BitAnd            = 29,   // x & y
    CdlExprOp_BitXor            = 30,   // x ^ y
    CdlExprOp_BitOr             = 31,   // x | y
    CdlExprOp_And               = 32,   // x && y
    CdlExprOp_Or                = 33,   // x || y
    CdlExprOp_Cond              = 34,   // x ? a : b
    CdlExprOp_StringConcat      = 35,   // x . y
    CdlExprOp_Implies           = 36,   // x implies y
    CdlExprOp_Xor               = 37,   // x xor y
    CdlExprOp_Eqv               = 38    // x eqv y
};

// ----------------------------------------------------------------------------
// A subexpression consists of an operation, possibly some constant
// data, and possibly indices into the subexpression vector.
// Normally some unions would be used, but unions and objects such
// as std::string do not mix, and the amount of memory involved is
// not big enough to really worry about.

#define CdlFunction_MaxArgs     3
struct CdlSubexpression {

    CdlExprOp           op;
    CdlSimpleValue      constants;              // String, integer or double constant
    int                 reference_index;        // iff CdlExprOp_Reference

    int                 lhs_index;              // for all non-constant operators
    int                 rhs_index;              // for binary and ternary operators only
    int                 rrhs_index;             // only for ternary operators.
    
    int                 func;                   // iff CdlExprOp_Function
    int                 args[CdlFunction_MaxArgs];
};

//}}}
//{{{  CdlFunction                      

// ----------------------------------------------------------------------------
// Generic support for function parsing, evaluation, and inference. The
// implementation is extensible so that functions can be added to the
// core via static constructors.

class CdlFunction {
    
    friend class CdlTest;
    
  public:
    CdlFunction(const char* /* name */, int /* no_args */,
                void (*)(CdlExpression, const CdlSubexpression&),
                void (*)(CdlEvalContext&, CdlExpression, const CdlSubexpression&, CdlSimpleValue&),
                bool (*)(CdlTransaction, CdlExpression, unsigned int, bool, int),
                bool (*)(CdlTransaction, CdlExpression, unsigned int, CdlSimpleValue&, int)
                );
    ~CdlFunction();
    
    static bool         is_function(std::string, int&);
    static std::string  get_name(int);
    static int          get_args_count(int);

    static void         check(CdlExpression, const CdlSubexpression&);
    static void         eval(CdlEvalContext&, CdlExpression, const CdlSubexpression&, CdlSimpleValue&);
    static bool         infer_bool(CdlTransaction, CdlExpression, unsigned int, bool, int);
    static bool         infer_value(CdlTransaction, CdlExpression, unsigned int, CdlSimpleValue&, int);

    static void         (*null_check)(CdlExpression, const CdlSubexpression&);
    static bool         (*null_infer_bool)(CdlTransaction, CdlExpression, unsigned int, bool, int);
    static bool         (*null_infer_value)(CdlTransaction, CdlExpression, unsigned int, CdlSimpleValue&, int);
    
  protected:

  private:
    // Keep track of all functions in the system
    static std::vector<CdlFunction*>    all_functions;

    // Each function object is given a unique id during initialization
    static int          next_id;
    int                 id;

    // Provided by the constructor
    const char*         name;
    int                 number_args;
    void                (*check_fn)(CdlExpression, const CdlSubexpression&);
    void                (*eval_fn)(CdlEvalContext&, CdlExpression, const CdlSubexpression&, CdlSimpleValue&);
    bool                (*infer_bool_fn)(CdlTransaction, CdlExpression, unsigned int, bool, int);
    bool                (*infer_value_fn)(CdlTransaction, CdlExpression, unsigned int, CdlSimpleValue&, int);
    
    // The default constructor is illegal
    CdlFunction();
};

//}}}
//{{{  CdlExpression                    

// ----------------------------------------------------------------------------
// And now for the expression class itself.

class CdlExpressionBody {

    friend class CdlTest;
    
  public:

    // The default constructor is basically a no-op, new expression
    // objects only get created as a consequence of parsing. However
    // it exists and is protected for the convenience of derived
    // classes. The copy constructor is protected, allowing parsing
    // code to first parse an expression and then copy the expression
    // into a higher level object. The assignment operator is illegal.
    // There is no reason to hide the destructor.
    virtual ~CdlExpressionBody();

    // An expression involves three pieces of data. There is a vector
    // of subexpressions. It is also necessary to know where
    // evaluation should being, in accordance with operator precedence
    // rules. And there is a vector of CdlReference objects - this
    // needs to be kept separate from the subexpression vector because
    // CdlReference objects are comparatively tricky.
    //
    // All of this data is public and can be readily inspected by the
    // inference engine, by conflict detection code, by diagnostic
    // code, etc.
    std::vector<CdlSubexpression>       sub_expressions;
    int                                 first_subexpression;
    std::vector<CdlReference>           references;
    bool                                update(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);
    
    // There are a number of parsing functions. The first one is
    // used by higher-level code to parse a single expression. Its
    // argument is a single string (which may be the result of
    // concatenating several Tcl arguments), and at the end of the
    // parse operation there should be no further data. The result
    // will either be a new expression object or a parsing exception
    // to be caught by higher level code.
    static CdlExpression        parse(std::string);

    // This is used when parsing list expressions, which involve a
    // sequence of ordinary expressions and possibly range operators.
    // The whole list expression lives in a single string, and it is
    // necessary to provide an index indicating where in the string
    // parsing should begin. It is also useful to return details of
    // the token that caused parsing to terminate (EOD, Range, or
    // the start of something else).
    static CdlExpression        parse(std::string, int&, CdlExprOp&, int&);

    // A goal expression is derived from an ordinary expression but
    // has somewhat different rules for evaluating. Parsing a goal
    // expression involves parsing a number of ordinary expressions
    // with implicit && operators between them, and it requires
    // a parsing function that can be used to extend an existing
    // expression.
    //
    // NOTE: possibly this should should be a protected member, since
    // its main use is in parsing goal expressions. 
    static void continue_parse(CdlExpression, std::string, int&, CdlExprOp&, int&);

    // Evaluating expressions. Note that this may fail at run-time
    // because of errors that cannot be caught sensibly when the
    // expression is read in, for example arithmetic overflow or
    // division by zero. Because such failures are a possibility
    // anyway no special action is taken to prevent an expression
    // with e.g. an unresolved reference from being evaluated.
    //
    // eval() is the public interface, and manages
    // CdlConflict_EvalException objects. eval_internal() is for use
    // by list and goal expressions.
    void eval(CdlEvalContext&, CdlSimpleValue&);
    void eval_internal(CdlEvalContext&, CdlSimpleValue&);
    void eval_subexpression(CdlEvalContext&, int, CdlSimpleValue&);
    
    // The full original expression is useful for diagnostics purposes
    std::string get_original_string() const;
    
    bool        check_this(cyg_assert_class_zeal cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:

    // The default constructor does very little, the main work
    // is done by the various parsing functions. However it is
    // available to derived classes, especially goal expressions.
    CdlExpressionBody();
    
    // The copy constructor has to be usable by derived classes,
    // e.g. CdlExpressionProperty
    CdlExpressionBody(const CdlExpressionBody&);
    
  private:

    
    // The assignment operator is illegal.
    CdlExpressionBody&  operator=(const CdlExpressionBody&);

    // The string that was parsed originally
    std::string                 expression_string;
    
    enum {
        CdlExpressionBody_Invalid       = 0,
        CdlExpressionBody_Magic         = 0x760293a3
    } cdlexpressionbody_cookie;
};

//}}}
//{{{  CdlListExpression                

// ----------------------------------------------------------------------------
// The main use of list expressions is for the legal_values
// properties. Essentially a list expression is just a vector of
// ordinary expressions and ranges of expressions. 

class CdlListExpressionBody {

    friend class CdlTest;

  public:

    // Availability of constructors etc. is as per the ordinary
    // expression class.
    virtual ~CdlListExpressionBody();
    
    // The data associated with a list expression is a vector of
    // expressions, plus a vector of expression pairs constituting
    // ranges. As with ordinary expressions the data is fully public
    // and can be readily examined by e.g. the inference engine.
    std::vector<CdlExpression>                                  data;
    std::vector<std::pair<CdlExpression,CdlExpression> >        ranges;

    // Parsing. This involves taking a single string, typically from
    // a CDL script, and parsing one or more ordinary expressions.
    static CdlListExpression parse(std::string);
    
    // Evaluation support. A list expression evaluates to a list value.
    void eval(CdlEvalContext&, CdlListValue&);

    // More commonly client code is going to be interested in whether
    // or not a particular value is a legal member. The result
    // cache ensures that it is possible to 
    bool is_member(CdlEvalContext&, CdlSimpleValue&);
    bool is_member(CdlEvalContext&, std::string);
    bool is_member(CdlEvalContext&, cdl_int);
    bool is_member(CdlEvalContext&, double);

    // The full original expression is useful for diagnostics purposes
    std::string get_original_string() const;
    
    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:
    CdlListExpressionBody(const CdlListExpressionBody&);
    bool        update(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);

  private:

    CdlListExpressionBody();
    CdlListExpressionBody& operator=(const CdlListExpressionBody&);

    void eval_internal(CdlEvalContext&, CdlListValue&);
    std::string         expression_string;
    
    enum {
        CdlListExpressionBody_Invalid   = 0,
        CdlListExpressionBody_Magic     = 0x7da4bcc2
    } cdllistexpressionbody_cookie;
};

//}}}
//{{{  CdlGoalExpression                

// ----------------------------------------------------------------------------
// A goal expression inherits privately from ordinary expressions. Essentially
// a goal expression is simply a set of ordinary expressions separated by &&,
// but it can only be evaluated to a boolean. The parse() and eval() members
// of the base class should not be exposed. There is a member to get hold of
// the underlying ordinary expression, for use by e.g. the inference engine.

class CdlGoalExpressionBody : private CdlExpressionBody {

    friend class CdlTest;

    typedef CdlExpressionBody inherited;

  public:
    virtual ~CdlGoalExpressionBody();

    static CdlGoalExpression parse(std::string);

    // A few variants of the eval() member, with a choice of returning
    // by value or by reference. The latter provide consistency with the
    // other expression classes.
    bool eval(CdlEvalContext&);
    void eval(CdlEvalContext&, bool&);

    // Provide public access to the underlying expression object,
    // useful for the inference engine
    CdlExpression               get_expression();
    
    // The full original expression is useful for diagnostics purposes
    std::string get_original_string() const;
    
    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:
    CdlGoalExpressionBody(const CdlGoalExpressionBody&);


  private:

    CdlGoalExpressionBody();
    CdlGoalExpressionBody& operator=(const CdlGoalExpressionBody&);
    void eval_internal(CdlEvalContext&, bool&);
    
    std::string expression_string;
    
    enum {
        CdlGoalExpressionBody_Invalid = 0,
        CdlGoalExpressionBody_Magic   = 0x5a58bb24
    } cdlgoalexpressionbody_cookie;
};

//}}}
//{{{  CdlInfer                         

// ----------------------------------------------------------------------------
// A utility class related to inference. This exports the main functions
// needed, allowing e.g. per-function inference routines from func.cxx to
// interact with the main inference engine.

class CdlInfer {
  public:
    static bool make_active(CdlTransaction, CdlNode, int /* level */);
    static bool make_inactive(CdlTransaction, CdlNode, int /* level */);
    static bool set_valuable_value(CdlTransaction, CdlValuable, CdlSimpleValue&, int /* level */);
    static bool set_valuable_bool(CdlTransaction, CdlValuable, bool, int /* level */);
    static bool subexpr_value(CdlTransaction, CdlExpression, unsigned int /* index */, CdlSimpleValue& goal, int /* level */);
    static bool subexpr_bool(CdlTransaction, CdlExpression, unsigned int /* index */, bool, int /* level */);

  private:
    CdlInfer();
};

//}}}

//}}}
//{{{  CdlConflict classes                              

// ----------------------------------------------------------------------------
// As a configuration is created and modified there will be times when
// things are not completely consistent. There may be a reference to
// some option that is not in any package in the current
// configuration. An option may have an invalid value, possibly as a
// side effect of a change to some other option. There may be a
// dependency that is not satisfied. There may be other types of
// conflict.
//
// The library provides a base class CdlConflict, and a number of
// derived classes for common types of conflict such as unresolved
// references. All conflicts are associated with a CdlNode and a
// property within that. It is possible to use dynamic_cast<> to find
// out the exact type of a conflict, or alternatively to use the
// virtual member function get_explanation().
//
// Conflicts may be disabled by the user if they are not actually
// important as far as application code is concerned. In other words
// the end user is allowed to override the constraints specified in
// the CDL. This information is saved with the configuration data.
// Preferably the user should give an explanation for why the conflict
// is disabled, to serve as a reminder some months later when the
// configuration is reloaded.
//
//
// Conflicts have a fairly complicated life cycle. First it is
// necessary to distinguish between structural and normal conflicts. A
// structural conflict is typically caused by a reference to a
// non-existent valuable. These conflicts are generally created only
// when something is loaded, and only go away when something is
// unloaded. A normal conflict is typically related to a value, for
// example a value outside the legal range, or a "requires" property
// that is not satisfied.
//
// Conflicts are created and destroyed in the context of a
// transaction, which in turn operates in the context of a toplevel.
// If the transaction is committed then new conflicts get added to the
// appropriate toplevel list, and destroyed conflicts get removed from
// the toplevel list. The transaction field indicates whether the
// conflict is currently per-transaction or global.
//
// Transactions may get nested, i.e. a conflict may get created as
// part of a sub-transaction, and when that sub-transaction is committed
// the conflict is moved to the parent transaction.
//
// For each toplevel, libcdl keeps track of all conflicts. This only
// applies to committed conflicts, per-transaction conflicts are not
// accessible in this way.
//
// As part of a transaction, libcdl may attempt to find solutions for
// particular conflicts, and those solutions may get installed
// automatically. No attempt is made to keep track of solutions
// on a global basis, only on a per-transaction basis.

class CdlConflictBody {
    
    friend class CdlTest;

    // Transactions and conflicts are closely connected
    friend class CdlTransactionBody;
    
  public:

    // Creation happens only inside a derived class.
    // Clearing a conflict only happens inside transactions.
    // Destroying a conflict only happens from inside a
    // per-transaction clear(), or during a transaction commit.
    
    // Is this conflict part of a transaction, or has it been committed to the toplevel.
    CdlTransaction      get_transaction() const;

    // Is inference implemented for this type of conflict?
    virtual bool        resolution_implemented() const;

    // Try to resolve an existing global conflict. A new transaction
    // is created for this operation, the conflict is resolved within
    // that transaction, and then CdlTransaction::body() is used to
    // handle inference callbacks, commits, etc. See also
    // CdlToplevel::resolve_conflicts() and
    // CdlToplevel::resolve_all_conflicts(). The conflict may cease to
    // exist as a side-effect of this call.
    void                resolve();
  
    // Keep track of whether or not this conflict has a solution
    // 1) a conflict may have a current solution. This gets invalidated
    //    whenever there is a change to a value that was referenced
    //    while identifying the solution.
    //
    //    A current solution is indicated by a non-empty solution vector.
    //    
    // 2) a conflict may not have a current solution. Again this gets
    //    invalidated whenever a referred value changes. There is a boolean
    //    to keep track of this.
    //
    // 3) a conflict may not have a current solution, but another run of
    //    the inference engine may find one.
    bool                has_known_solution() const;
    bool                has_no_solution() const;
    const std::vector<std::pair<CdlValuable, CdlValue> >& get_solution() const;
    const std::set<CdlValuable>& get_solution_references() const;
    void                clear_solution();
    
    // Provide a text message "explaining" the conflict.
    // This only makes sense for derived classes.
    virtual std::string get_explanation() const = 0;

    // Basic information access.
    CdlNode             get_node() const;
    CdlProperty         get_property() const;
    bool                is_structural() const;

    // Enabling and disabling conflicts currently happens outside the
    // context of any transaction.
    // FIXME: these are not currently implemented. It would be necessary
    // to store the information in the savefile, which requires an
    // unambiguous way of identifying a conflict that is likely to
    // survice package version changes.
    void                disable(std::string);
    void                enable();
    bool                is_enabled() const;
    std::string         get_disabled_reason() const;
    
    bool check_this(cyg_assert_class_zeal zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:
    CdlConflictBody(CdlTransaction, CdlNode, CdlProperty, bool /* structural */);
    
    // The destructor gets accessed from inside the friend transaction class,
    // either during a clear_conflict() or during a transaction commit.
    virtual ~CdlConflictBody();
    
    // All conflicts are associated with a node and a property.
    // This information will be useful to derived classes'
    // implementations of get_explanation()
    CdlNode             node;
    CdlProperty         property;
    
  private:

    // Attempt to resolve a conflict in a sub-transaction
    // This is invoked from inside the transaction resolve code.
    // There are additional exported interfaces inside and outside
    // the transaction class.
    virtual bool        inner_resolve(CdlTransaction, int);
    
    // Keep track of the transaction in which this conflict was created.
    // The field is cleared at the end of a transaction.
    CdlTransaction      transaction;

    // Usually the derived class will decide whether or not
    // this conflict is structural in nature, but the data
    // needs to be available at base constructor time so
    // a virtual function is not appropriate.
    bool                structural;
    
    // Solution support
    bool                                           no_solution;
    std::vector<std::pair<CdlValuable, CdlValue> > solution;
    std::set<CdlValuable>                          solution_references;
    void update_solution_validity(CdlValuable);
    
    // Users may disable a conflict. Usually they will have to
    // supply a reason for this.
    bool                enabled;
    std::string         reason;

    enum {
        CdlConflictBody_Invalid = 0,
        CdlConflictBody_Magic   = 0x073e8853
    } cdlconflictbody_cookie;

    // Illegal operations. Conflicts always live on the heap.
    CdlConflictBody();
    CdlConflictBody(const CdlConflictBody&);
    CdlConflictBody& operator=(const CdlConflictBody&);
};

// ----------------------------------------------------------------------------
// An unresolved conflict means that there is a reference in some
// property to an entity that is not yet in the current configuration.
// The class provides convenient access to the name of the unresolved
// entity. 

class CdlConflict_UnresolvedBody : public CdlConflictBody {
    
    friend class CdlTest;
  public:

    static void         make(CdlTransaction, CdlNode, CdlProperty, std::string);

    std::string         get_target_name() const;
    std::string         get_explanation() const;
    static bool         test(CdlConflict);
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:

  private:
    virtual ~CdlConflict_UnresolvedBody();
    CdlConflict_UnresolvedBody(CdlTransaction, CdlNode, CdlProperty, std::string);
    std::string         target_name;
    enum {
        CdlConflict_UnresolvedBody_Invalid      = 0,
        CdlConflict_UnresolvedBody_Magic        = 0x1b24bb8a
    } cdlconflict_unresolvedbody_cookie;
    
    CdlConflict_UnresolvedBody();
    CdlConflict_UnresolvedBody(const CdlConflict_UnresolvedBody&);
    CdlConflict_UnresolvedBody& operator=(const CdlConflict_UnresolvedBody&);
};

// ----------------------------------------------------------------------------
// An illegal value can be caused because of a number of properties:
// legal_values, check_proc, entry_proc, ... In the case of the latter
// the Tcl code should provide text explaining why the value is
// illegal.

class CdlConflict_IllegalValueBody : public CdlConflictBody {

    friend class CdlTest;

  public:

    static void         make(CdlTransaction, CdlNode, CdlProperty);

    bool                resolution_implemented() const;
    
    std::string         get_explanation() const;
    void                set_explanation(std::string);
    static bool         test(CdlConflict);
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:

  private:
    virtual ~CdlConflict_IllegalValueBody();
    bool    inner_resolve(CdlTransaction, int);
    CdlConflict_IllegalValueBody(CdlTransaction, CdlNode, CdlProperty);
    std::string explanation;
    enum {
        CdlConflict_IllegalValueBody_Invalid    = 0,
        CdlConflict_IllegalValueBody_Magic      = 0x4fb27ed1
    } cdlconflict_illegalvaluebody_cookie;

    CdlConflict_IllegalValueBody();
    CdlConflict_IllegalValueBody(const CdlConflict_IllegalValueBody&);
    CdlConflict_IllegalValueBody& operator=(const CdlConflict_IllegalValueBody&);
};

// ----------------------------------------------------------------------------
// There are times when expression evaluation will fail, e.g. because of
// a division by zero. The explanation is supplied by the evaluation code.

class CdlConflict_EvalExceptionBody : public CdlConflictBody {

    friend class CdlTest;

  public:

    static void         make(CdlTransaction, CdlNode, CdlProperty, std::string);

    std::string         get_explanation() const;
    void                set_explanation(std::string);   // mainly for internal use
    static bool         test(CdlConflict);
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:

  private:
    virtual ~CdlConflict_EvalExceptionBody();
    CdlConflict_EvalExceptionBody(CdlTransaction, CdlNode, CdlProperty, std::string);
    std::string explanation;
    enum {
        CdlConflict_EvalExceptionBody_Invalid   = 0,
        CdlConflict_EvalExceptionBody_Magic     = 0x7e64bc41
    } cdlconflict_evalexceptionbody_cookie;
};

// ----------------------------------------------------------------------------
// A goal expression evaluates to false. Producing sensible diagnostics
// depends on a detailed understanding of goal expressions, which will 
// have to wait until the inference engine comes along.

class CdlConflict_RequiresBody : public CdlConflictBody {

    friend class CdlTest;

  public:

    static void         make(CdlTransaction, CdlNode, CdlProperty);
    bool                resolution_implemented() const;
    
    std::string         get_explanation() const;
    static bool         test(CdlConflict);
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:

  private:
    virtual ~CdlConflict_RequiresBody();
    bool     inner_resolve(CdlTransaction, int);
    CdlConflict_RequiresBody(CdlTransaction, CdlNode, CdlProperty);
    enum {
        CdlConflict_RequiresBody_Invalid        = 0,
        CdlConflict_RequiresBody_Magic          = 0x78436331
    } cdlconflict_requiresbody_cookie;
};

// ----------------------------------------------------------------------------
// There is an unusual problem in the configuration data somewhere.
// For example, a parent property can be resolved but the target is
// not a container. There is not a lot that the user can do about
// problems like this, apart from complaining to the component vendor,
// but the problem should not be ignored either.

class CdlConflict_DataBody : public CdlConflictBody {

    friend class CdlTest;

  public:

    static void         make(CdlTransaction, CdlNode, CdlProperty, std::string);

    std::string         get_explanation() const;
    static bool         test(CdlConflict);
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:

  private:
    virtual ~CdlConflict_DataBody();
    CdlConflict_DataBody(CdlTransaction, CdlNode, CdlProperty, std::string);
    std::string message;
    enum {
        CdlConflict_DataBody_Invalid    = 0,
        CdlConflict_DataBody_Magic      = 0x2cec7ad8
    } cdlconflict_databody_cookie;
};

//}}}
//{{{  CdlProperty class and derived classes            

//{{{  Description                              

// ---------------------------------------------------------------------------
// There are many different kinds of property. An alias property contains
// a simple string. A check_proc property contains a fragment of Tcl code
// which can be represented internally as a string, as bytecodes, or both.
// A requires property contains a goal expression. ...
//
// The implementation involves a base class CdlProperty and various
// derived classes such as CdlProperty_StringBody and
// CdlProperty_ExpressionBody.
//
// New CdlProperty objects get created only when reading in CDL scripts,
// while executing commands like alias and requires. These commands are
// implemented as C++ functions hooked into the TCL interpreter. The
// property arguments are available as an argc/argv pair. Each command
// will parse and validate the arguments and then invoke an appropriate
// constructor.

//}}}
//{{{  CdlPropertyId_xxx                        

// ----------------------------------------------------------------------------
// Properties are identified by strings rather than by an enum or anything
// like that. A string-based approach allows new properties to be added at
// any time without invalidating an existing enum, complicating switch()
// statements, etc. There are some performance issues but these are
// manageable.
//
// A disadvantage of using strings is that there is a problem with
// typos. Mistyping something like CdlPropertyId_Compile will generally
// result in a compile-time failure. Mistyping "Complie" will cause
// strange behaviour at run-time and is hard to track down.
//
// A compromise solution is to have #define'd string constants.

#define CdlPropertyId_ActiveIf          "ActiveIf"
#define CdlPropertyId_BuildProc         "BuildProc"
#define CdlPropertyId_Calculated        "Calculated"
#define CdlPropertyId_CancelProc        "CancelProc"
#define CdlPropertyId_CheckProc         "CheckProc"
#define CdlPropertyId_Compile           "Compile"
#define CdlPropertyId_ConfirmProc       "ConfirmProc"
#define CdlPropertyId_DecorationProc    "DecorationProc"
#define CdlPropertyId_DefaultValue      "DefaultValue"
#define CdlPropertyId_Define            "Define"
#define CdlPropertyId_DefineHeader      "DefineHeader"
#define CdlPropertyId_DefineProc        "DefineProc"
#define CdlPropertyId_Description       "Description"
#define CdlPropertyId_Dialog            "Dialog"
#define CdlPropertyId_Display           "Display"
#define CdlPropertyId_DisplayProc       "DisplayProc"
#define CdlPropertyId_Doc               "Doc"
#define CdlPropertyId_EntryProc         "EntryProc"
#define CdlPropertyId_Flavor            "Flavor"
#define CdlPropertyId_DefineFormat      "DefineFormat"
#define CdlPropertyId_Group             "Group"
#define CdlPropertyId_Hardware          "Hardware"
#define CdlPropertyId_IfDefine          "IfDefine"
#define CdlPropertyId_Implements        "Implements"
#define CdlPropertyId_IncludeDir        "IncludeDir"
#define CdlPropertyId_IncludeFiles      "IncludeFiles"
#define CdlPropertyId_InitProc          "InitProc"
#define CdlPropertyId_InstallProc       "InstallProc"
#define CdlPropertyId_LegalValues       "LegalValues"
#define CdlPropertyId_Library           "Library"
#define CdlPropertyId_LicenseProc       "LicenseProc"
#define CdlPropertyId_Make              "Make"
#define CdlPropertyId_Makefile          "Makefile"
#define CdlPropertyId_MakeObject        "MakeObject"
#define CdlPropertyId_NoDefine          "NoDefine"
#define CdlPropertyId_Object            "Object"
#define CdlPropertyId_Parent            "Parent"
#define CdlPropertyId_Requires          "Requires"
#define CdlPropertyId_Screen            "Screen"
#define CdlPropertyId_Script            "Script"
#define CdlPropertyId_UpdateProc        "UpdateProc"
#define CdlPropertyId_Wizard            "Wizard"

//}}}
//{{{  Base class                               

// ----------------------------------------------------------------------------
// The base class is never used directly. Instead the appropriate derived
// objects are instantiated and when appropriate it will be necessary to
// do a dynamic cast from a CdlProperty to e.g. a CdlProperty_String.

class CdlPropertyBody {

    friend class CdlTest;
    
  public:
    // The destructor is public, to avoid possible problems with STL.
    virtual ~CdlPropertyBody();
    
    // These routines provide access to the basic data.
    std::string get_property_name() const;

    // Get hold of the arguments that were present in the original data.
    int         get_argc() const;
    bool        has_option(std::string) const;
    std::string get_option(std::string) const;
    const std::vector<std::string>&     get_argv() const;
    const std::vector<std::pair<std::string,std::string> >&     get_options() const;
    
    // Resolve any references, or generate/update appropriate conflict
    // objects. The default implementation is a no-op because not all
    // properties involve references.
    virtual void update(CdlTransaction, CdlNode /* source */, CdlNode /* dest */, CdlUpdate);
    
    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:

    // The legal constructor can only get invoked from a derived class
    // constructor. The first argument identifies the property, e.g.
    // CdlPropertyId_Doc (which is just #define'd to the string
    // "doc").
    //
    // The argc and argv fields provide access to the original
    // data in the command that resulted in the property being
    // constructed. Often but not always argv[0] will be the same as
    // the property id. The argv information is stored mainly for
    // diagnostics purposes, it may be removed in future to avoid
    // wasting memory.
    //
    // The options field is the result of parsing options such
    // as -library=libextras.a. It consists of a vector of
    // <name/value> pairs, and is usually obtained via
    // CdlParse::parse_options().
    CdlPropertyBody(CdlNode, std::string, int argc, const char* argv[], std::vector<std::pair<std::string,std::string> >&);
    
  private:
    // This string indicates the command used to define this property,
    // e.g. "doc" or "define_proc". It is provided to the constructor.
    std::string                 name;

    // All property data comes out of a file and gets rid via a
    // Tcl interpreter. The raw file data is stored with the property,
    // mainly for diagnostics purposes.
    std::vector<std::string>    argv;

    std::vector<std::pair<std::string, std::string> >   options;

    // The usual set of illegal operations.
    CdlPropertyBody();
    CdlPropertyBody(const CdlPropertyBody&);
    CdlPropertyBody& operator=(const CdlPropertyBody&);
    
    enum {
        CdlPropertyBody_Invalid = 0,
        CdlPropertyBody_Magic   = 0x60dd58f4
    } cdlpropertybody_cookie;
};

//}}}
//{{{  CdlProperty_Minimal                      

// ----------------------------------------------------------------------------
// This class is used for properties that are simple flags, e.g. no_define.
// There should be no additional data associated with such properties.

class CdlProperty_MinimalBody : public CdlPropertyBody {

    friend class CdlTest;
    
  public:
    static CdlProperty_Minimal   make(CdlNode, std::string, int, const char*[], std::vector<std::pair<std::string,std::string> >&);
    virtual ~CdlProperty_MinimalBody( );
    bool                        check_this( cyg_assert_class_zeal = cyg_quick ) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:
    
  private:
    typedef CdlPropertyBody     inherited;
    
    CdlProperty_MinimalBody(CdlNode, std::string, int, const char*[], std::vector<std::pair<std::string,std::string> >&);
    enum {
        CdlProperty_MinimalBody_Invalid = 0,
        CdlProperty_MinimalBody_Magic   = 0x25625b8c
    } cdlproperty_minimalbody_cookie;

    CdlProperty_MinimalBody();
    CdlProperty_MinimalBody(const CdlProperty_MinimalBody&);
    CdlProperty_MinimalBody& operator=(const CdlProperty_MinimalBody&);
};

//}}}
//{{{  CdlProperty_String                       

// ----------------------------------------------------------------------------
// A string property contains a single piece of additional data in the form
// of a string.

class CdlProperty_StringBody : public CdlPropertyBody {

    friend class CdlTest;
    
  public:
    static CdlProperty_String    make(CdlNode, std::string, std::string, int, const char*[],
                                      std::vector<std::pair<std::string,std::string> >&);
    virtual ~CdlProperty_StringBody();

    std::string                 get_string(void) const;
    bool                        check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:
    
  private:
    typedef CdlPropertyBody     inherited;
    
    CdlProperty_StringBody(CdlNode, std::string /* id */, std::string /* data */, int, const char*[],
                           std::vector<std::pair<std::string,std::string> >&);
    std::string                 data;
    enum {
        CdlProperty_StringBody_Invalid = 0,
        CdlProperty_StringBody_Magic   = 0x78d1ca94
    } cdlproperty_stringbody_cookie;

    // The only legal constructor supplies all the data.
    CdlProperty_StringBody();
    CdlProperty_StringBody(const CdlProperty_StringBody&);
    CdlProperty_StringBody& operator=(const CdlProperty_StringBody&);
};

//}}}
//{{{  CdlProperty_TclCode                      

// ----------------------------------------------------------------------------
// A TclCode property is currently equivalent to a string property. In
// future this may change to allow the byte-compiled versions of the
// script to be stored.
//
// One of the properties, "screen" inside a cdl_wizard, also takes
// an integer. Rather than create yet another class, this is handled
// by a separate constructor.


class CdlProperty_TclCodeBody : public CdlPropertyBody {

    friend class CdlTest;

  public:
    static CdlProperty_TclCode   make(CdlNode, std::string, cdl_tcl_code, int, const char*[],
                                      std::vector<std::pair<std::string,std::string> >&);
    static CdlProperty_TclCode   make(CdlNode, std::string, cdl_int, cdl_tcl_code, int, const char*[],
                                      std::vector<std::pair<std::string,std::string> >&);
    virtual ~CdlProperty_TclCodeBody();
    
    cdl_int                     get_number(void) const;
    const cdl_tcl_code&         get_code(void)   const;
    bool                        check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  private:
    typedef CdlPropertyBody     inherited;
    
    CdlProperty_TclCodeBody(CdlNode, std::string, cdl_int, cdl_tcl_code, int, const char*[],
                            std::vector<std::pair<std::string,std::string> >&);

    cdl_int                     number;
    cdl_tcl_code                code;
    enum {
        CdlProperty_TclCodeBody_Invalid = 0,
        CdlProperty_TclCodeBody_Magic   = 0x7b14d4e5
    } cdlproperty_tclcodebody_cookie;

    CdlProperty_TclCodeBody();
    CdlProperty_TclCodeBody(const CdlProperty_TclCodeBody&);
    CdlProperty_TclCodeBody& operator=(const CdlProperty_TclCodeBody&);
    
};

//}}}
//{{{  CdlProperty_StringVector                 

// ----------------------------------------------------------------------------
// This is used for multiple constant strings, as opposed to a list
// expression which requires evaluation. One example is a list
// of aliases.

class CdlProperty_StringVectorBody : public CdlPropertyBody {

    friend class CdlTest;

  public:
    static CdlProperty_StringVector     make(CdlNode, std::string, const std::vector<std::string>&, int, const char*[],
                                             std::vector<std::pair<std::string,std::string> >&);
    virtual ~CdlProperty_StringVectorBody();
    
    const std::vector<std::string>&     get_strings() const;
    std::string                         get_first_string() const;
    unsigned int                        get_number_of_strings() const;
    std::string                         get_string(unsigned int) const;                  
    bool                                check_this(cyg_assert_class_zeal zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  private:
    typedef CdlPropertyBody            inherited;
    
    CdlProperty_StringVectorBody(CdlNode, std::string, const std::vector<std::string>&, int, const char*[],
                                 std::vector<std::pair<std::string,std::string> >&);

    std::vector<std::string>            data;
    enum {
        CdlProperty_StringVectorBody_Invalid = 0,
        CdlProperty_StringVectorBody_Magic   = 0x4ed039f3
    } cdlproperty_stringvectorbody_cookie;

    CdlProperty_StringVectorBody();
    CdlProperty_StringVectorBody(const CdlProperty_StringVectorBody&);
    CdlProperty_StringVectorBody& operator=(const CdlProperty_StringVectorBody&);
};

//}}}
//{{{  CdlProperty_Reference                    

// ----------------------------------------------------------------------------
// This is used for properties such as wizard and dialog, where the data
// identifies some other entity in the system. The class is both a property
// and a reference object. Most of the desired functionality is provided by
// inheritance from CdlReference.

class CdlProperty_ReferenceBody : public CdlPropertyBody, public CdlReference {

    friend class CdlTest;

  public:
    static CdlProperty_Reference make(CdlNode, std::string /* id */, std::string /* destination */,
                                      CdlUpdateHandler, int, const char*[],
                                      std::vector<std::pair<std::string,std::string> >&);
    virtual ~CdlProperty_ReferenceBody();
    
    void update(CdlTransaction, CdlNode, CdlNode, CdlUpdate);
    
    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  private:
    typedef CdlPropertyBody     inherited_property;
    typedef CdlReference        inherited_reference;

    CdlUpdateHandler            update_handler;
    
    CdlProperty_ReferenceBody(CdlNode, std::string /* id */, std::string /* destination */, CdlUpdateHandler, int, const char*[],
                              std::vector<std::pair<std::string,std::string> >&);
    enum {
        CdlProperty_ReferenceBody_Invalid = 0,
        CdlProperty_ReferenceBody_Magic   = 0x78100339
    } cdlproperty_referencebody_cookie;

    CdlProperty_ReferenceBody();
    CdlProperty_ReferenceBody(const CdlProperty_ReferenceBody&);
    CdlProperty_ReferenceBody& operator=(const CdlProperty_Reference&);
};

//}}}
//{{{  CdlProperty_Expression                   

// ----------------------------------------------------------------------------
// An expression property simply inherits its functionality from the basic
// property class and from the expression class.

class CdlProperty_ExpressionBody : public CdlPropertyBody, public CdlExpressionBody {

    friend class CdlTest;
    
  public:
    static CdlProperty_Expression       make(CdlNode, std::string, CdlExpression, CdlUpdateHandler, int, const char*[],
                                             std::vector<std::pair<std::string,std::string> >&);
    virtual ~CdlProperty_ExpressionBody();
    void update(CdlTransaction, CdlNode, CdlNode, CdlUpdate);
    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  private:
    typedef CdlPropertyBody     inherited_property;
    typedef CdlExpressionBody   inherited_expression;

    CdlProperty_ExpressionBody(CdlNode, std::string, CdlExpression, CdlUpdateHandler, int, const char*[],
                               std::vector<std::pair<std::string,std::string> >&);
    
    CdlUpdateHandler update_handler;
    enum {
        CdlProperty_ExpressionBody_Invalid = 0,
        CdlProperty_ExpressionBody_Magic   = 0x05fb4056
    } cdlproperty_expressionbody_cookie;

    CdlProperty_ExpressionBody();
    CdlProperty_ExpressionBody(const CdlProperty_ExpressionBody&);
    CdlProperty_ExpressionBody& operator=(const CdlProperty_ExpressionBody&);
};

//}}}
//{{{  CdlProperty_ListExpression               

// ----------------------------------------------------------------------------
// Similarly a list property simply inherits from property and from
// list expressions.

class CdlProperty_ListExpressionBody : public CdlPropertyBody, public CdlListExpressionBody {

    friend class CdlTest;
    
  public:
    static CdlProperty_ListExpression   make(CdlNode, std::string, CdlListExpression, CdlUpdateHandler, int, const char*[],
                                             std::vector<std::pair<std::string,std::string> >&);
    virtual ~CdlProperty_ListExpressionBody();
    void update(CdlTransaction, CdlNode, CdlNode, CdlUpdate);
    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  private:
    typedef CdlPropertyBody         inherited_property;
    typedef CdlListExpressionBody   inherited_expression;

    CdlProperty_ListExpressionBody(CdlNode, std::string, CdlListExpression, CdlUpdateHandler, int, const char*[],
                                   std::vector<std::pair<std::string,std::string> >&);

    CdlUpdateHandler update_handler;
    enum {
        CdlProperty_ListExpressionBody_Invalid = 0,
        CdlProperty_ListExpressionBody_Magic   = 0x6b0136f5
    } cdlproperty_listexpressionbody_cookie;

    CdlProperty_ListExpressionBody();
    CdlProperty_ListExpressionBody(const CdlProperty_ListExpressionBody&);
    CdlProperty_ListExpressionBody& operator=(const CdlProperty_ListExpressionBody&);
};

//}}}
//{{{  CdlProperty_GoalExpression               

// ----------------------------------------------------------------------------
// And a goal property inherits from property and from goal expressions.

class CdlProperty_GoalExpressionBody : public CdlPropertyBody, public CdlGoalExpressionBody {

    friend class CdlTest;
    
  public:
    static CdlProperty_GoalExpression   make(CdlNode, std::string, CdlGoalExpression, CdlUpdateHandler, int, const char*[],
                                             std::vector<std::pair<std::string,std::string> >&);
    virtual ~CdlProperty_GoalExpressionBody();
    void update(CdlTransaction, CdlNode, CdlNode, CdlUpdate);
    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  private:
    typedef CdlPropertyBody         inherited_property;
    typedef CdlGoalExpressionBody   inherited_expression;

    CdlProperty_GoalExpressionBody(CdlNode, std::string, CdlGoalExpression, CdlUpdateHandler, int, const char*[],
                                   std::vector<std::pair<std::string,std::string> >&);
    
    CdlUpdateHandler update_handler;
    enum {
        CdlProperty_GoalExpressionBody_Invalid = 0,
        CdlProperty_GoalExpressionBody_Magic   = 0x08b2b31e
    } cdlproperty_goalexpressionbody_cookie;

    CdlProperty_GoalExpressionBody();
    CdlProperty_GoalExpressionBody(const CdlProperty_GoalExpressionBody&);
    CdlProperty_GoalExpressionBody& operator=(const CdlProperty_GoalExpressionBody&);
};

//}}}

//}}}
//{{{  CdlParse class                                   

// ----------------------------------------------------------------------------
// This is another utility class for collecting together parsing-related
// functions.
//
// Note that this is only a utility class. When libcdl is used for parsing
// things not related to software configuration the new functionality
// does not have to reside inside the CdlParse class, but it may be
// possible to re-use some of the functionality in that class.

class CdlParse {
    
  public:
    // Utility routines.
    static std::string  get_tcl_cmd_name(std::string);
    static std::string  concatenate_argv(int, const char*[], int);
    static int          parse_options(CdlInterpreter, std::string /* diag_prefix */, char** /* options */,
                                               int /* argc */, const char*[] /* argv */, int /* start_index */,
                                               std::vector<std::pair<std::string,std::string> >& /* result */);
    static std::string  construct_diagnostic(CdlInterpreter, std::string /* classification */,
                                             std::string /* sub-identifier */, std::string /* message */);
                                       
    static void         report_error(CdlInterpreter, std::string /* sub-identifier */, std::string /* message */);
    static void         report_warning(CdlInterpreter, std::string /* sub-identifier */, std::string /* message */);
    static void         clear_error_count(CdlInterpreter);
    static int          get_error_count(CdlInterpreter);
    static void         incr_error_count(CdlInterpreter, int=1);

    static std::string  get_expression_error_location(void);
    
    // Support for Tcl's "unknown" command
    static int          unknown_command(CdlInterpreter, int, const char*[]);
    
    // Property-related utilities
    static void         report_property_parse_error(CdlInterpreter, std::string, std::string);
    static void         report_property_parse_error(CdlInterpreter, CdlProperty, std::string);
    static void         report_property_parse_warning(CdlInterpreter, std::string, std::string);
    static void         report_property_parse_warning(CdlInterpreter, CdlProperty, std::string);
    
    // Utility parsing routines
    static int  parse_minimal_property(CdlInterpreter, int, const char*[], std::string,
                                       char**, void (*)(CdlInterpreter, CdlProperty_Minimal));
    static int  parse_string_property(CdlInterpreter, int, const char*[], std::string,
                                      char**, void (*)(CdlInterpreter, CdlProperty_String));
    static int  parse_tclcode_property(CdlInterpreter, int, const char*[], std::string,
                                       char**, void (*)(CdlInterpreter, CdlProperty_TclCode));
    static int  parse_stringvector_property(CdlInterpreter, int, const char*[], std::string,
                                            char**, void (*)(CdlInterpreter, CdlProperty_StringVector),
                                            bool /* allow_empty */ = false);
    static int  parse_reference_property(CdlInterpreter, int, const char*[], std::string,
                                         char**, void (*)(CdlInterpreter, CdlProperty_Reference),
                                         bool /* allow_empty */,
                                         CdlUpdateHandler);
    static int  parse_expression_property(CdlInterpreter, int, const char*[], std::string, 
                                          char **, void (*)(CdlInterpreter, CdlProperty_Expression),
                                          CdlUpdateHandler);
    static int  parse_listexpression_property(CdlInterpreter, int, const char*[], std::string,
                                              char **, void (*)(CdlInterpreter, CdlProperty_ListExpression),
                                              CdlUpdateHandler);
    static int  parse_goalexpression_property(CdlInterpreter, int, const char*[], std::string,
                                              char **, void (*)(CdlInterpreter, CdlProperty_GoalExpression),
                                              CdlUpdateHandler);
};

//}}}
//{{{  CdlNode                                          

// ----------------------------------------------------------------------------
// A node object has a name and lives in a hierarchy. Each node keeps
// track of the toplevel and owner. The memory overheads are
// relatively small compared with the performance gains when such
// information is needed.
//
// A node object also has a vector of properties, and can be referred to
// by properties in other nodes. Some of the properties may result in
// conflicts.

class CdlNodeBody {

    friend class CdlTest;

    // Adding and removing nodes from the hierarchy is done
    // by CdlToplevel members.
    friend class CdlToplevelBody;

    // CdlLoadable must be able to access the destructor
    friend class CdlLoadableBody;
    
    // It is intended that CdlProperties will also add and remove themselves
    friend class CdlPropertyBody;

    // CdlReference bind and unbind operations need access to
    // the referrers vector. So does CdlTransaction::commit()
    friend class CdlReference;
    friend class CdlTransactionBody;
    
  public:

    // Basic information.
    std::string         get_name() const;
    CdlContainer        get_parent() const;
    CdlLoadable         get_owner() const;
    CdlToplevel         get_toplevel() const;

    // Propagation support. Some updates such as active/inactive changes
    // get applied to nodes as well as to properties. Note that because
    // of multiple inheritance this virtual call can get confusing.
    virtual void        update(CdlTransaction, CdlUpdate);
    
    // Is this node active or not? The is_active() call refers
    // to the global state, things may be different inside a
    // transaction.
    bool                is_active() const;
    bool                is_active(CdlTransaction transaction);

    // Generally nodes become active when the parent becomes
    // active and enabled. Some derived classes may impose
    // additional restrictions, for example because of
    // active_if constraints. This routine can be used
    // to check whether or not a node should become active.
    virtual bool        test_active(CdlTransaction);
    
    // Provide access to the various properties. Currently all this
    // information is publicly available.
    const std::vector<CdlProperty>&     get_properties() const;
    CdlProperty                         get_property(std::string) const;
    void                                get_properties(std::string, std::vector<CdlProperty>&) const;
    std::vector<CdlProperty>            get_properties(std::string) const;
    bool                                has_property(std::string) const;
    int                                 count_properties(std::string) const;

    // Provide access to the various global conflicts. More
    // commonly conflicts are accessed on a per-transaction basis.
    void get_conflicts(std::vector<CdlConflict>&) const;
    void get_conflicts(bool (*)(CdlConflict), std::vector<CdlConflict>&) const;
    void get_structural_conflicts(std::vector<CdlConflict>&) const;
    void get_structural_conflicts(bool (*)(CdlConflict), std::vector<CdlConflict>&) const;
    
    // Provide access to all the referrers. This may not get used very
    // much outside the library itself.
    const std::vector<CdlReferrer>&     get_referrers() const;

    // Add property parsers and validation code appropriate for a
    // node. Currently this is a no-op, there are no properties
    // associated with every node, but this may change in future e.g.
    // for diagnostics purposes.
    static void add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers);
    void        check_properties(CdlInterpreter);

    // Persistence support. The final classes such as cdl_option
    // should provide implementations of these functions. The
    // base function takes care of data that was present in an
    // original save file but which was not recognised.
    //
    // Configuration save files are Tcl scripts, so it seems
    // appropriate to handle the I/O via the Tcl library and
    // to have a TCL interpreter available.
    virtual void save(CdlInterpreter, Tcl_Channel, int, bool);
    bool has_additional_savefile_information() const;
    
    // Mainly for diagnostics code, what is the actual name for this
    // type of CDL object? This should be in terms of CDL data, e.g.
    // "package" or "component", rather than in implementation terms
    // such as "CdlPackageBody". 
    virtual std::string get_class_name() const;

    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:
    
    // CdlNodeBodies are only instantiated by derived classes.
    // They must always have a name. They need not be placed
    // in the hierarchy immediately, that can wait until
    // later.
    CdlNodeBody(std::string);
    // A dummy constructor is needed because of the virtual
    // inheritance. 
    CdlNodeBody();

    // Nodes cannot be destroyed directly by application code,
    // only by higher-level library functions such as unload_package()
    virtual ~CdlNodeBody();

    // Updating the name is rarely required, but is useful for savefiles.
    void                set_name(std::string);
    
    // Is the node currently active? This applies to the global state
    // only, not per-transaction state. Some derived classes may want
    // to override the default value
    bool                active;
    
  private:
    
    // The basic data. The name is known during construction.
    // The other three fields get updated by e.g. CdlToplevel::add_node();
    std::string         name;
    CdlContainer        parent;
    CdlLoadable         owner;
    CdlToplevel         toplevel;

    // This is used by remove_node_from_toplevel()/add_node_to_toplevel()
    // to allow the latter to exactly reverse the former
    int                 remove_node_container_position;
    
    // Properties normally only get added during the parsing process,
    // and only get removed when the object itself is destroyed.
    // A vector is the obvious implementation.
    std::vector<CdlProperty> properties;

    // Currently a vector of referrers is used. This vector is subject
    // to change when packages get loaded and unloaded, possibly a
    // list would be better.
    std::vector<CdlReferrer> referrers;

    // Savefiles may contain information that is not recognised by the
    // current library, especially because of savefile hooks which
    // allow e.g. the configuration tool to store its own information
    // in save files. This information must not be lost, even if you are
    // e.g. mixing command line and GUI tools. This vector holds
    // the savefile information so that it can be put in the next
    // savefile.
    std::vector<std::string> unsupported_savefile_strings;
    
    enum {
        CdlNodeBody_Invalid     = 0,
        CdlNodeBody_Magic       = 0x309595b5
    } cdlnodebody_cookie;
    
    // Illegal operations
    CdlNodeBody(const CdlNodeBody&);
    CdlNodeBody& operator=(const CdlNodeBody&);
};

//}}}
//{{{  CdlContainer                                     

// ----------------------------------------------------------------------------
// A container is a node that can contain other nodes.

class CdlContainerBody : virtual public CdlNodeBody {

    friend class Cdltest;

    // Allow CdlNode::check_this() access to the internals
    friend class CdlNodeBody;
    
    // Adding a node to the hierarchy is done by a CdlToplevel member.
    // Ditto for removing.
    friend class CdlToplevelBody;

    // Deleting a container can happen inside CdlToplevel and CdlLoadable
    friend class CdlLoadableBody;
    
  public:

    const std::vector<CdlNode>& get_contents() const;
    bool                        contains(CdlConstNode, bool /* recurse */ = false) const;
    bool                        contains(const std::string, bool /* recurse */ = false) const;
    CdlNode                     find_node(const std::string, bool /* recurse */ = false) const;

    // Propagation support. Some updates such as active/inactive changes
    // get applied to nodes as well as to properties.
    virtual void update(CdlTransaction, CdlUpdate);
    
    // Persistence support.
    virtual void save(CdlInterpreter, Tcl_Channel, int, bool);
    
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:

    // Containers cannot be destroyed explicitly, only via higher-level
    // code such as unload_package();
    virtual ~CdlContainerBody();

    CdlContainerBody();
    // Special constructor needed for internal use.
    CdlContainerBody(std::string);

    // The CdlToplevel class needs access to its own contents.
    std::vector<CdlNode>                contents;
    
  private:
    enum {
        CdlContainerBody_Invalid        = 0,
        CdlContainerBody_Magic          = 0x543c5f1d
    } cdlcontainerbody_cookie;

    // Illegal operations
    CdlContainerBody(const CdlContainerBody&);
    CdlContainerBody& operator=(const CdlContainerBody&);
};

//}}}
//{{{  CdlLoadable                                      

// ----------------------------------------------------------------------------
// A loadable object is a container that gets loaded or unloaded
// atomically from a toplevel. The key difference is that a loadable
// keeps track of all nodes that were loaded as part of this
// operation, thus allowing unload operations to happen safely even if
// nodes get re-parented all over the hierarchy. In addition, there is
// a slave interpreter associated with every loadable.

class CdlLoadableBody : virtual public CdlContainerBody {

    friend class CdlTest;

    // Allow CdlNode::check_this() access to the internals
    friend class CdlNodeBody;
    
    // Adding nodes to the hierarchy is done by a toplevel member
    friend class CdlToplevelBody;

  public:
    virtual ~CdlLoadableBody();

    
    const std::vector<CdlNode>&         get_owned() const;
    bool                                owns(CdlConstNode) const;
    CdlInterpreter                      get_interpreter() const;
    std::string                         get_directory() const;

    // Some properties such as doc and compile reference filenames.
    // A search facility is useful.
    virtual std::string find_relative_file(std::string /* filename */, std::string /* directory */ = "") const;
    virtual std::string find_absolute_file(std::string, std::string, bool /* allow_urls */ = false) const;
    virtual bool        has_subdirectory(std::string) const;

    // These support load/unload operations inside transactions
    // They are static members because some of them will want
    // to delete the loadable.
    static void         transaction_commit_load(CdlTransaction, CdlLoadable);
    static void         transaction_cancel_load(CdlTransaction, CdlLoadable);
    static void         transaction_commit_unload(CdlTransaction, CdlLoadable);
    static void         transaction_cancel_unload(CdlTransaction, CdlLoadable);

    // Binding and unbinding of properties. This involves processing
    // the various properties, calculating default values, etc.
    void                bind(CdlTransaction);
    void                unbind(CdlTransaction);
    
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:

    CdlLoadableBody(CdlToplevel, std::string /* directory */);

    // Needed by derived classes, but not actually used.
    CdlLoadableBody();
    
  private:
    
    std::vector<CdlNode> owned;
    CdlInterpreter       interp;
    std::string          directory;

    // Used by add/remove_node_from_toplevel()
    int                  remove_node_loadables_position;
    
    enum {
        CdlLoadableBody_Invalid = 0,
        CdlLoadableBody_Magic   = 0x488d6127
    } cdlloadablebody_cookie;

    // Invalid operations
    CdlLoadableBody(const CdlLoadableBody&);
    CdlLoadableBody& operator=(const CdlLoadableBody&);
};

//}}}
//{{{  CdlToplevel                                      

// ----------------------------------------------------------------------------
// Toplevels are containers that live at the top of a hierarchy
// (surprise surprise). This means that they do not have a parent.
// In addition a toplevel object keeps track of all the names
// used, guaranteeing uniqueness and providing a quick lookup
// facility.
//
// Every container is also a node, so every toplevel is a node.
// Inheritance from CdlNode may seem wrong. However it achieves
// consistency, everything in the hierarchy including the toplevel
// is a node. The main disadvantage is that every toplevel now
// needs a name.

class CdlToplevelBody : virtual public CdlContainerBody {

    friend class CdlTest;

    // Allow CdlNode::check_this() access to the internals
    friend class CdlNodeBody;
    
    // The CdlTransaction class needs direct access to the lists
    // of conflicts.
    friend class CdlTransactionBody;

  public:
    virtual ~CdlToplevelBody();

    // Updating the hierarchy. This happens a node at a time. Adding a
    // node involves updating the name->node map in the toplevel,
    // setting the node's parent/owner/toplevel fields, and updating
    // the parent and owner containers. The owner may be 0 for special
    // nodes such as the orphans container. The parent must be known,
    // although it may change later on during a change_parent() call.
    //
    // Removing a node is more complicated, and involves a two-stage
    // process. First the node is removed from the toplevel, thus
    // eliminating the name->node mapping. The owner and parent fields
    // are preserved at this stage (except for the loadable itself),
    // and the operation may be undone if the relevant transaction
    // gets cancelled. If the transaction gets committed then the
    // second remove operation handles the owner and parent fields,
    // just prior to the node being deleted. For convenience there
    // are also per-loadable variants for some of these.
    //
    // change_parent() is used to support parent-properties.
    // A container of 0 indicates an orphan, i.e. a parent
    // property that did not or does not correspond to a
    // current container.
    //
    // There is also a clean-up call. This gets used for interfaces
    // which may alternate between belonging to a loadable and
    // being auto-created.
    void add_node(CdlLoadable, CdlContainer, CdlNode);
    void add_node_to_toplevel(CdlNode);
    void remove_node_from_toplevel(CdlNode);
    static void remove_node(CdlLoadable, CdlContainer, CdlNode);
    void add_loadable_to_toplevel(CdlLoadable);
    void remove_loadable_from_toplevel(CdlLoadable);
    void change_parent(CdlLoadable, CdlContainer /* current */, CdlContainer /* new */, CdlNode, int /* pos */ = -1);
    void cleanup_orphans();

    // Toplevels keep track of all the loadables, in addition to
    // inheriting tree behaviour from CdlContainer. This is convenient
    // for some operations like determining build information
    // which must operate on a per-loadable basis.
    const std::vector<CdlLoadable>& get_loadables() const;
    
    // Name uniqueness is guaranteed. It is convenient to have an STL
    // map as a lookup service.
    CdlNode lookup(const std::string) const;

    // There are two conflict lists associated with each toplevel. One
    // is for "structural" conflicts, ones that can only be resolved
    // by a fairly major change such as loading another package: a
    // typical example is an unresolved parent reference. The other is
    // for conflicts that can probably be resolved simply by changing
    // some values. Both sets of conflicts are held as a simple list.
    //
    // The active vs. inactive state of a CDL entity affects the
    // location of structural vs. non-structural conflicts. If an
    // entity becomes inactive then structural conflicts are not
    // affected, but non-structural conflicts are removed from the
    // global list. If an entity's "requires" expression is not
    // satisfied but the entity is inactive anyway then this is
    // harmless.
    const std::list<CdlConflict>& get_all_conflicts() const;
    const std::list<CdlConflict>& get_all_structural_conflicts() const;

    // Try to resolve some or all conflicts. Typically a new transaction
    // will be created for this.
    void        resolve_conflicts(const std::vector<CdlConflict>&);
    void        resolve_all_conflicts();
    
    // Toplevels can have descriptions provided by the user. This is
    // particularly important for pre-defined templates, target
    // board descriptions, etc. where the user would like some
    // extra information about the template before loading it in.
    // The default value is an empty string.
    std::string         get_description() const;
    void                set_description(std::string);
    
    // Each toplevel must have an associated master Tcl interpreter.
    CdlInterpreter      get_interpreter() const;

    // Each toplevel should also have an associated directory for 
    // the component repository. It is not required that all loadables
    // are relative to this, but that is the default behaviour.
    std::string         get_directory() const;

    // Each toplevel may have a single active main transaction.
    // For now there is no support for concurrent transactions
    // operating on a single toplevel (although nested transactions
    // are allowed)
    CdlTransaction      get_active_transaction() const;
    
    // Build and define operations are available for all toplevels,
    // even if they are not always applicable
    void                get_build_info(CdlBuildInfo&);
    void                get_all_build_info(CdlBuildInfo&);
    void                generate_config_headers(std::string);
    void                get_config_headers(std::vector<std::string>&);
    void                generate_build_tree(std::string, std::string = "");
    
    // Values can be stored in limbo. This is useful when unloading 
    // and reloading packages, e.g. when changing a version the
    // current settings can be preserved as much as possible.
    void                set_limbo_value(CdlValuable);
    bool                has_limbo_value(std::string) const;
    CdlValue            get_limbo_value(std::string) const;
    CdlValue            get_and_remove_limbo_value(std::string);
    void                clear_limbo();
    
    // Persistence support. These are commented in the source code.
           void         initialize_savefile_support();
    static bool         savefile_support_initialized();
           void         add_savefile_command(std::string, CdlSaveCallback, CdlInterpreterCommand);
           void         add_savefile_subcommand(std::string, std::string, CdlSaveCallback, CdlInterpreterCommand);
           void         get_savefile_commands(std::vector<CdlInterpreterCommandEntry>&);
           void         get_savefile_subcommands(std::string, std::vector<CdlInterpreterCommandEntry>&);
           void         save_command_details(CdlInterpreter, Tcl_Channel, int, bool);
    static int          savefile_handle_command(CdlInterpreter, int, const char*[]);
    static int          savefile_handle_unsupported(CdlInterpreter, int, const char*[]);
    static int          savefile_handle_unknown(CdlInterpreter, int, const char*[]);
           void         save_unsupported_commands(CdlInterpreter, Tcl_Channel, int, bool);
    static cdl_int      get_library_savefile_version();
    static int          savefile_handle_version(CdlInterpreter, int, const char*[]);
    static cdl_int      get_savefile_version(CdlInterpreter);
           void         save_conflicts(CdlInterpreter, Tcl_Channel, int, bool);
    static void         save_separator(CdlInterpreter, Tcl_Channel, std::string, bool);
        
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  protected:
    CdlToplevelBody(CdlInterpreter, std::string);

  private:

    std::map<std::string,CdlNode>       lookup_table;
    std::vector<CdlLoadable>            loadables;
    std::map<std::string,CdlValue>      limbo;
    CdlInterpreter                      interp;
    CdlContainer                        orphans;
    std::string                         description;
    std::string                         directory;
    std::list<CdlConflict>              conflicts;
    std::list<CdlConflict>              structural_conflicts;

    // The savefile support corresponding to this application.
    static cdl_int savefile_version;
    static bool    savefile_commands_initialized;
    static std::vector<CdlSavefileCommand> savefile_commands;
    static std::map<std::string,std::vector<CdlSavefileCommand> > savefile_subcommands;

    // Per-toplevel support. A savefile may contain unrecognised
    // commands at the toplevel of a file, as well as unrecognised
    // commands in e.g. the body of a cdl_configuration command.
    // The latter is handled via the CdlNode base class.
    std::vector<std::string> unsupported_savefile_toplevel_strings;
    std::vector<std::string> unsupported_savefile_commands;
    std::map<std::string, std::vector<std::string> > unsupported_savefile_subcommands;

    // Keep track of the current active transaction for this toplevel (if any)
    CdlTransaction      transaction;
    
    enum {
        CdlToplevelBody_Invalid = 0,
        CdlToplevelBody_Magic   = 0x0834666e
    } cdltoplevelbody_cookie;
    
    // Invalid operations
    CdlToplevelBody(const CdlToplevelBody&);
    CdlToplevelBody& operator=(const CdlToplevelBody&);
};

//}}}
//{{{  CdlUserVisible                                   

// ----------------------------------------------------------------------------
// A user-visible object is likely to have properties such as display,
// description and doc. Many user-visible objects will have values but
// not all, for example custom dialogs are likely to have a doc
// property but they do not have a value.

class CdlUserVisibleBody : virtual public CdlNodeBody {

    friend class CdlTest;

  public:
    virtual ~CdlUserVisibleBody();

    std::string         get_display() const;
    std::string         get_description() const;
    std::string         get_doc() const;

    // NOTE: this will only work for absolute doc strings or for doc
    // strings that are relative to the package.
    std::string         get_doc_url() const;
    
    // Add property parsers and validation code appropriate for a
    // user-visible object such as doc and description 
    static void         add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers);
    void                check_properties(CdlInterpreter);
    static int          parse_description(CdlInterpreter, int, const char*[]);
    static int          parse_display(CdlInterpreter, int, const char*[]);
    static int          parse_doc(CdlInterpreter, int, const char*[]);

    // Persistence support. The save code simply outputs some comments
    // corresponding to the display, doc and description properties.
    virtual void save(CdlInterpreter, Tcl_Channel, int, bool);
    
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:
    CdlUserVisibleBody();

  private:

    enum {
        CdlUserVisibleBody_Invalid      = 0,
        CdlUserVisibleBody_Magic        = 0x13bbc817
    } cdluservisiblebody_cookie;

    // Illegal operations
    CdlUserVisibleBody(const CdlUserVisibleBody&);
    CdlUserVisibleBody& operator=(const CdlUserVisibleBody&);
};

//}}}
//{{{  CdlParentable                                    

// ----------------------------------------------------------------------------
// A parentable object may have the parent property, redefining its
// position in the hierarchy.

class CdlParentableBody : virtual public CdlNodeBody {

    friend class CdlTest;

  public:
    virtual ~CdlParentableBody();

    static void         add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers);
    void                check_properties(CdlInterpreter);
    static int          parse_parent(CdlInterpreter, int, const char*[]);
    static void         update_handler(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);

    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:
    CdlParentableBody();

  private:

    // Unloads may be cancelled. To restore the previous state exactly
    // it is necessary to keep track of the old position.
    int                 change_parent_save_position;
    
    enum {
        CdlParentableBody_Invalid      = 0,
        CdlParentableBody_Magic        = 0x40c6a077
    } cdlparentablebody_cookie;

    // Illegal operations
    CdlParentableBody(const CdlParentableBody&);
    CdlParentableBody& operator=(const CdlParentableBody&);
};

//}}}
//{{{  CdlValuable                                      

// ----------------------------------------------------------------------------
// A valuable body has a value. Many valuables can be modified but not all.
// Some properties make a valuable object read-only. In future there is
// likely to be support for locked values as well. There is a member function
// to check whether or not a valuable object is modifiable.
//
// Relevant properties for a valuable object are:
//
//  1) flavor           - readily available via CdlValue::get_flavor()
//  2) default_value    - an expression
//  3) legal_values     - a list expression
//  4) entry_proc       - for validation purposes, in addition to legal_values
//  5) check_proc       - ditto
//  6) active_if        - goal expression
//  7) requires         - goal expression
//  8) dialog           - a custom dialog for editing this value
//  9) calculated       - non-modifiable
// 10) implements       - for interfaces
//
// A CdlValuable does not inherit directly from CdlValue, since it should
// not be possible to modify a Valuable directly. Instead it contains a
// CdlValue member, and provides essentially the same functions as
// a CdlValue.

class CdlValuableBody : virtual public CdlNodeBody {

    friend class CdlTest;

    // Transaction commit operations require direct access to the CdlValue
    friend class CdlTransactionBody;
    
  private:
    CdlValue value;
    
  public:
    virtual ~CdlValuableBody();

    // Accessing the current value. There are variants for the global state
    // and for per-transaction operations.
    const CdlValue&     get_whole_value() const;
    
    CdlValueFlavor      get_flavor() const;
    CdlValueFlavor      get_flavor(CdlTransaction transaction) const
    {   // The transaction is irrelevant, it cannot change the flavor
        return this->get_flavor();
    }

    CdlValueSource      get_source() const;
    bool                has_source(        CdlValueSource) const;
    bool                is_enabled(        CdlValueSource = CdlValueSource_Current) const;
    std::string         get_value(         CdlValueSource = CdlValueSource_Current) const;
    bool                has_integer_value( CdlValueSource = CdlValueSource_Current) const;
    cdl_int             get_integer_value( CdlValueSource = CdlValueSource_Current) const;
    bool                has_double_value(  CdlValueSource = CdlValueSource_Current) const;
    double              get_double_value(  CdlValueSource = CdlValueSource_Current) const;
    CdlSimpleValue      get_simple_value(  CdlValueSource = CdlValueSource_Current) const;
    
    CdlValueSource      get_source(CdlTransaction) const;
    bool                has_source(        CdlTransaction, CdlValueSource) const;
    bool                is_enabled(        CdlTransaction, CdlValueSource = CdlValueSource_Current) const;
    std::string         get_value(         CdlTransaction, CdlValueSource = CdlValueSource_Current) const;
    bool                has_integer_value( CdlTransaction, CdlValueSource = CdlValueSource_Current) const;
    cdl_int             get_integer_value( CdlTransaction, CdlValueSource = CdlValueSource_Current) const;
    bool                has_double_value(  CdlTransaction, CdlValueSource = CdlValueSource_Current) const;
    double              get_double_value(  CdlTransaction, CdlValueSource = CdlValueSource_Current) const;
    CdlSimpleValue      get_simple_value(  CdlTransaction, CdlValueSource = CdlValueSource_Current) const;
    
    // -----------------------------------------------------------------
    // Modify access. There are two variants of all the functions:
    //
    // 1) no transaction argument. A transaction will be created,
    //    committed, and destroyed for the change in question.
    //
    // 2) a transaction argument. The existing transaction will be
    //    updated but not committed. This allows multiple changes
    //    to be grouped together.
    //
    // There are only a handful of exported functions, but lots
    // of inline variants.
    void set_source(CdlValueSource);
    void invalidate_source(CdlValueSource);
    void set_enabled(bool, CdlValueSource);
    void set_value(CdlSimpleValue&, CdlValueSource);
    void set_enabled_and_value(bool, CdlSimpleValue&, CdlValueSource);
    void set(CdlSimpleValue&, CdlValueSource);
    
    void set_source(CdlTransaction, CdlValueSource);
    void invalidate_source(CdlTransaction, CdlValueSource);
    void set_enabled(CdlTransaction, bool, CdlValueSource);
    void set_value(CdlTransaction, CdlSimpleValue&, CdlValueSource);
    void set_enabled_and_value(CdlTransaction, bool, CdlSimpleValue&, CdlValueSource);
    void set(CdlTransaction, CdlSimpleValue&, CdlValueSource);
    void set(CdlTransaction, const CdlValue&);
    
    void enable(CdlValueSource source)
    {
        set_enabled(true, source);
    }
    void disable(CdlValueSource source)
    {
        set_enabled(false, source);
    }
    void set_value(std::string data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_value(val, source);
    }
    void set_integer_value(cdl_int data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_value(val, source);
    }
    void set_double_value(double data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_value(val, source);
    }
    void set_enabled_and_value(bool enabled, std::string data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_enabled_and_value(enabled, val, source);
    }
    void set_enabled_and_value(bool enabled, cdl_int data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_enabled_and_value(enabled, val, source);
    }
    void set_enabled_and_value(bool enabled, double data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_enabled_and_value(enabled, val, source);
    }
    void enable_and_set_value(CdlSimpleValue& val, CdlValueSource source)
    {
        set_enabled_and_value(true, val, source);
    }
    void enable_and_set_value(std::string data, CdlValueSource source)
    {
        set_enabled_and_value(true, data, source);
    }
    void enable_and_set_value(cdl_int data, CdlValueSource source)
    {
        set_enabled_and_value(true, data, source);
    }
    void enable_and_set_value(double data, CdlValueSource source)
    {
        set_enabled_and_value(true, data, source);
    }
    void disable_and_set_value(CdlSimpleValue& val, CdlValueSource source)
    {
        set_enabled_and_value(false, val, source);
    }
    void disable_and_set_value(std::string data, CdlValueSource source)
    {
        set_enabled_and_value(false, data, source);
    }
    void disable_and_set_value(cdl_int data, CdlValueSource source)
    {
        set_enabled_and_value(false, data, source);
    }
    void disable_and_set_value(double data, CdlValueSource source)
    {
        set_enabled_and_value(false, data, source);
    }
    void enable(CdlTransaction transaction, CdlValueSource source)
    {
        set_enabled(transaction, true, source);
    }
    void disable(CdlTransaction transaction, CdlValueSource source)
    {
        set_enabled(transaction, false, source);
    }
    void set_value(CdlTransaction transaction, std::string data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_value(transaction, val, source);
    }
    void set_integer_value(CdlTransaction transaction, cdl_int data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_value(transaction, val, source);
    }
    void set_double_value(CdlTransaction transaction, double data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_value(transaction, val, source);
    }
    void set_enabled_and_value(CdlTransaction transaction, bool enabled, std::string data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_enabled_and_value(transaction, enabled, val, source);
    }
    void set_enabled_and_value(CdlTransaction transaction, bool enabled, cdl_int data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_enabled_and_value(transaction, enabled, val, source);
    }
    void set_enabled_and_value(CdlTransaction transaction, bool enabled, double data, CdlValueSource source)
    {
        CdlSimpleValue val(data);
        set_enabled_and_value(transaction, enabled, val, source);
    }
    void enable_and_set_value(CdlTransaction transaction, CdlSimpleValue& val, CdlValueSource source)
    {
        set_enabled_and_value(transaction, true, val, source);
    }
    void enable_and_set_value(CdlTransaction transaction, std::string data, CdlValueSource source)
    {
        set_enabled_and_value(transaction, true, data, source);
    }
    void enable_and_set_value(CdlTransaction transaction, cdl_int data, CdlValueSource source)
    {
        set_enabled_and_value(transaction, true, data, source);
    }
    void enable_and_set_value(CdlTransaction transaction, double data, CdlValueSource source)
    {
        set_enabled_and_value(transaction, true, data, source);
    }
    void disable_and_set_value(CdlTransaction transaction, CdlSimpleValue& val, CdlValueSource source)
    {
        set_enabled_and_value(transaction, false, val, source);
    }
    void disable_and_set_value(CdlTransaction transaction, std::string data, CdlValueSource source)
    {
        set_enabled_and_value(transaction, false, data, source);
    }
    void disable_and_set_value(CdlTransaction transaction, cdl_int data, CdlValueSource source)
    {
        set_enabled_and_value(transaction, false, data, source);
    }
    void disable_and_set_value(CdlTransaction transaction, double data, CdlValueSource source)
    {
        set_enabled_and_value(transaction, false, data, source);
    }

    // -----------------------------------------------------------------
    virtual bool   is_modifiable() const;
    void           get_widget_hint(CdlWidgetHint&);

    // -----------------------------------------------------------------
    // Propagation support. If a valuable becomes active or inactive
    // because e.g. its parent is disabled then this may affect
    // requires conflicts etc.
    virtual void update(CdlTransaction, CdlUpdate);
    
    virtual bool test_active(CdlTransaction);
    
    // -----------------------------------------------------------------
    // Property-related stuff.
    bool                                has_calculated_expression() const;
    bool                                has_default_value_expression() const;
    bool                                has_legal_values() const;
    bool                                has_entry_proc() const;
    bool                                has_check_proc() const;
    bool                                has_active_if_conditions() const;
    bool                                has_requires_goals() const;
    bool                                has_dialog() const;
    bool                                has_wizard() const;
    
    CdlProperty_Expression              get_calculated_expression() const;
    CdlProperty_Expression              get_default_value_expression() const;
    CdlProperty_ListExpression          get_legal_values() const;
    cdl_tcl_code                        get_entry_proc() const;
    cdl_tcl_code                        get_check_proc() const;
    void                                get_active_if_conditions(std::vector<CdlProperty_GoalExpression>&) const;
    void                                get_requires_goals(std::vector<CdlProperty_GoalExpression>&) const;
    CdlDialog                           get_dialog() const;
    CdlWizard                           get_wizard() const;
    void                                get_implemented_interfaces(std::vector<CdlInterface>&) const;
    
    // Add property parsers and validation code appropriate for a
    // valuable object such as default_value and legal_values
    static void         add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers);
    void                check_properties(CdlInterpreter);
    static int          parse_active_if(CdlInterpreter, int, const char*[]);
    static void         active_if_update_handler(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);
    static int          parse_calculated(CdlInterpreter, int, const char*[]);
    static void         calculated_update_handler(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);
    static int          parse_check_proc(CdlInterpreter, int, const char*[]);
    static int          parse_default_value(CdlInterpreter, int, const char*[]);
    static void         default_value_update_handler(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);
    static int          parse_dialog(CdlInterpreter, int, const char*[]);
    static void         dialog_update_handler(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);
    static int          parse_entry_proc(CdlInterpreter, int, const char*[]);
    static int          parse_flavor(CdlInterpreter, int, const char*[]);
    static int          parse_group(CdlInterpreter, int, const char*[]);
    static int          parse_implements(CdlInterpreter, int, const char*[]);
    static void         implements_update_handler(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);
    static int          parse_legal_values(CdlInterpreter, int, const char*[]);
    static void         legal_values_update_handler(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);
    static int          parse_requires(CdlInterpreter, int, const char*[]);
    static void         requires_update_handler(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);
    static int          parse_wizard(CdlInterpreter, int, const char*[]);
    static void         wizard_update_handler(CdlTransaction, CdlNode, CdlProperty, CdlNode, CdlUpdate);

    // Persistence suppot
    void save(CdlInterpreter, Tcl_Channel, int, bool /* modifiable */, bool /* minimal */);
    bool value_savefile_entry_needed() const;
    static void initialize_savefile_support(CdlToplevel, std::string);
    static int  savefile_value_source_command(CdlInterpreter, int, const char*[]);
    static int  savefile_user_value_command(CdlInterpreter, int, const char*[]);
    static int  savefile_wizard_value_command(CdlInterpreter, int, const char*[]);
    static int  savefile_inferred_value_command(CdlInterpreter, int, const char*[]);
    static int  savefile_xxx_value_command(CdlInterpreter, int, const char*[], CdlValueSource);
    
    // Make sure that the current value is legal. This gets called automatically
    // by all the members that modify values. It has to be a virtual function
    // since some derived classes, e.g. hardware-related valuables, may impose
    // constraints over and above legal_values etc.
    virtual void check_value(CdlTransaction);

    // Similarly check the requires properties
    void check_requires(CdlTransaction, CdlProperty_GoalExpression);
    void check_requires(CdlTransaction);

    // Enabling or disabling a valuable may affect the active state of children
    void check_children_active(CdlTransaction);

    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:
    CdlValuableBody(CdlValueFlavor = CdlValueFlavor_Bool);

    
  private:

    
    enum {
        CdlValuableBody_Invalid = 0,
        CdlValuableBody_Magic   = 0x2b2acc03
    } cdlvaluablebody_cookie;

    // Illegal operations
    CdlValuableBody(const CdlValuableBody&);
    CdlValuableBody& operator=(const CdlValuableBody&);
};

//}}}
//{{{  CdlTransaction etc.                              

//{{{  Description                      

// ----------------------------------------------------------------------------
// Transactions. These are used for all changes to a configuration. In some
// cases a transaction is implicit:
//
//    valuable->set_value(...)
//
// The actual implementation of this is:
//
//    valuable->set_value(...)
//        transact = CdlTransactionBody::make(valuable->get_toplevel())
//        valuable->set_value(transact, ...)
//        <complicated bits>
//        transact->commit()
//        delete transact
//
// Alternatively the use of transactions may be explicit. For implicit
// uses the library will invoke an inference callback at the
// appropriate time. For explicit transactions this is not necessary.
//
// The commit() operation invokes a transaction callback which should
// not be confused with the inference callback. The former is intended
// for display updates, it specifies everything that has changed
// during the transaction. The latter is used for reporting new
// conflicts to the user, suggesting fixes, etc.
//
// A whole bunch of information is associated with a transaction,
// including: all value changes, details of new conflicts, and details
// of existing conflicts that have gone away. The commit operation
// takes care of updating the toplevel. Until the commit happens
// the toplevel itself remains unchanged. It is also possible to cancel
// a transaction.
//
// An important concept related to transactions is propagation.
// Changing a value may have various effects, for example it may
// change the result of a legal_values list expression, resulting in a
// conflict object having to be created or destroyed. Changing one
// value may result in other value changes, e.g. because of a
// default_value property. All this is "propagation", and may
// happen multiple times within a single transaction.
//
// Transaction objects are also used during load or unload operations,
// but those are a little bit special. In particular it is not possible
// to cancel such a transaction, there will have been updates to the
// toplevel. Using a transaction is convenient because there is a 
// need for propagation.
//
// Currently a transaction should be deleted immediately after a
// commit or cancel. This may change in future, in that transaction
// objects can be used to hold undo information.
//
//
// The other big concept related to transactions is inference.
// Changing a value may result in one or more new conflicts being
// created. In some cases the library can figure out for itself how to
// resolve these conflicts, using an inference engine. There are
// parameters to control the operation of the inference engine,
// including whether it runs at all, what changes it is allowed
// to make automatically (usually default and inferred values can
// be updated, but not wizard or user values), and how much
// recursion will happen.
//
// Assuming a default setup in a GUI environment, a typical
// sequence of events would be:
//
//     valuable->set_value(...)
//         transact = CdlTransactionBody::make(valuable->get_toplevel())
//         valuable->set_value(transact, ...)
//             transact->set_whole_value(valuable, ...)
//         transact->propagate()
//         while (!finished)
//             transact->resolve()
//                 <inference>
//             invoke inference callback
//                 transact->apply_solution() (1 or more times)
//                     transact->set_whole_value(valuable, ...) (1 or more times)
//             transact->propagate()
//         transact->commit() | transact->cancel()
//         delete transact
//
// Note that the propagation steps have to be invoked explicitly,
// allowing multiple changes to be processed in one go. There is
// a utility function which combines the functionality from
// the first propagate() call up to but not including the
// transaction delete operator. 
//
//
// The inference engine itself is a complicated beast. There are
// a number of interfaces, but at the end of the day it ends up
// creating a sub-transaction and trying to resolve a single
// conflict in that sub-transaction. The conflict may belong to
// the current transaction or it may be global.
//
//     <inference>
//         for each conflict of interest
//             make sure that there is not already a valid solution
//             check that the inference engine can handle it
//             create a sub-transaction, associated with the conflict
//             apply the conflict resolution code
//             if the solution is ok
//                 install it
//             else if the solution might e.g. overwrite a user value
//                 keep it, the user can decide during the inference callback
//
// The conflict resolution typically works by attempting to change
// one or more values in the sub-transaction, propagating them,
// and seeing what new conflicts get created. If no new conflicts
// get created and one or more existing conflicts go away, groovy.
// Otherwise recursion can be used to try to resolve the new
// conflicts, or other strategies can be explored.
//
// NOTE: what is really necessary is some way of keeping track of the
// "best" solution to date, and allow exploration of alternatives.
// Or possibly keep track of all solutions. That has to be left to
// a future version.

//}}}
//{{{  CdlTransactionCommitCancelOp     

// ----------------------------------------------------------------------------
// The CdlTransaction class has built-in knowledge of how to handle values,
// active state, and a few things like that. However there are also more
// complicated operations such as loading and unloading, instantiating
// items, etc. which also need to happen in the context of a transaction
// but which the transaction class does not necessarily know about
// itself - or at least, not in any detail. Since the libcdl core is
// intended to be useful in various contexts, some sort of extensibility
// is essential.
//
// This is achieved by an auxiliary class, CdlTransactionCommitCancelOp.
// Clients of the transaction class can have their own utility class
// which derives from this, and create suitable objects. The transaction
// class maintains a vector of the pending commit/cancel operations.
//
// Each CdlTransactionCommitCancelOp object has two member functions,
// one for when the transaction gets committed and one for when it
// gets cancelled. If a sub-transaction gets committed then its
// pending ops are transferred across to the parent, allowing the
// parent to be cancelled sensibly: the commit ops only get run for
// the toplevel transaction. If a sub-transaction gets cancelled then
// the pending ops are invoked immediately.
//
// There is an assumption that commit/cancel ops get executed strictly
// in FIFO order. Specifically, commit ops get run from first one to
// the last one, allowing later operations in the transaction to
// overwrite earlier ones. Cancel ops get run in reverse order.

class CdlTransactionCommitCancelOp {
    friend class CdlTest;

  public:

    CdlTransactionCommitCancelOp() { }
    virtual ~CdlTransactionCommitCancelOp() { };

    // The default implementations of both of these do nothing.
    // Derived classes should override at least one of these
    // functions.
    virtual void commit(CdlTransaction transaction) {
        CYG_UNUSED_PARAM(CdlTransaction, transaction);
    }
    virtual void cancel(CdlTransaction transaction) {
        CYG_UNUSED_PARAM(CdlTransaction, transaction);
    }
    
  protected:

  private:
};

//}}}
//{{{  CdlTransaction class             

class CdlTransactionBody {

    friend class CdlTest;

    friend class CdlConflictBody;
    friend class CdlValuableBody;
    
  public:

    // Create a toplevel transaction
    static CdlTransaction make(CdlToplevel);
    virtual ~CdlTransactionBody();
    CdlToplevel get_toplevel() const;
    
    // Or a sub-transaction. Usually these are created in the context of
    // a conflict that is being resolved.
    CdlTransaction make(CdlConflict = 0);
    CdlTransaction get_parent() const;
    CdlConflict    get_conflict() const;
    
    // Commit all the changes. Essentially this means transferring
    // all of the per-transaction data to the toplevel, and then
    // invoking the transaction callback. All propagation, inference,
    // etc. should happen before the commit()
    // This routine can also be used to transfer changes from a
    // sub-transaction to the parent.
    void        commit();

    // A variant of the commit() operation can be used to
    // store a sub-transaction in a conflict's solution vector,
    // rather than updating the parent transaction. This is useful
    // for inferred solutions which cannot be applied without
    // user confirmation
    void        save_solution();

    // Can a solution held in a sub-transaction be applied without
    // e.g. overwriting a user value with an inferred value?
    bool        user_confirmation_required() const;

    // If the user has explicitly changed a value in the current transaction
    // then the inference engine should not undo this or suggest a solution
    // that will undo the change.
    bool        changed_by_user(CdlValuable) const;

    // A variant which is used for checking the hierarchy when disabling
    // a container
    bool        subnode_changed_by_user(CdlContainer) const;
    
    // Is one transaction preferable to another?
    bool        is_preferable_to(CdlTransaction) const;
    
    // Find out about per-transaction conflicts. This is particularly
    // useful for the inference callback. The other containers can
    // be accessed as well, for completeness.
    const std::list<CdlConflict>&       get_new_conflicts() const;
    const std::list<CdlConflict>&       get_new_structural_conflicts() const;
    const std::vector<CdlConflict>&     get_deleted_conflicts() const;
    const std::vector<CdlConflict>&     get_deleted_structural_conflicts() const;
    const std::vector<CdlConflict>&     get_resolved_conflicts() const ;
    const std::list<CdlConflict>&       get_global_conflicts_with_solutions() const;
    const std::map<CdlValuable, CdlValue>& get_changes() const;
    const std::set<CdlNode>&            get_activated() const;
    const std::set<CdlNode>&            get_deactivated() const;
    const std::set<CdlValuable>&        get_legal_values_changes() const;
    
    // Manipulate the current set of conflicts, allowing for nested
    // transactions and toplevel conflicts as well.
    void        clear_conflict(CdlConflict);
    bool        has_conflict_been_cleared(CdlConflict);
    
    bool        has_conflict(CdlNode, bool (*)(CdlConflict));
    CdlConflict get_conflict(CdlNode, bool (*)(CdlConflict));
    void        get_conflicts(CdlNode, bool (*)(CdlConflict), std::vector<CdlConflict>&);
    void        clear_conflicts(CdlNode, bool (*)(CdlConflict));
    bool        has_conflict(CdlNode, CdlProperty, bool (*)(CdlConflict));
    CdlConflict get_conflict(CdlNode, CdlProperty, bool (*)(CdlConflict));
    void        get_conflicts(CdlNode, CdlProperty, bool (*)(CdlConflict), std::vector<CdlConflict>&);
    void        clear_conflicts(CdlNode, CdlProperty, bool (*)(CdlConflict));

    bool        has_structural_conflict(CdlNode, bool (*)(CdlConflict));
    CdlConflict get_structural_conflict(CdlNode, bool (*)(CdlConflict));
    void        get_structural_conflicts(CdlNode, bool (*)(CdlConflict), std::vector<CdlConflict>&);
    void        clear_structural_conflicts(CdlNode, bool (*)(CdlConflict));
    bool        has_structural_conflict(CdlNode, CdlProperty, bool (*)(CdlConflict));
    CdlConflict get_structural_conflict(CdlNode, CdlProperty, bool (*)(CdlConflict));
    void        get_structural_conflicts(CdlNode, CdlProperty, bool (*)(CdlConflict), std::vector<CdlConflict>&);
    void        clear_structural_conflicts(CdlNode, CdlProperty, bool (*)(CdlConflict));

    // During the inference callback the user may decide to
    // apply one or more of the solutions.
    void        apply_solution(CdlConflict);
    void        apply_solutions(const std::vector<CdlConflict>&);
    void        apply_all_solutions();
    
    // Cancel all the changes done in this transaction. Essentially
    // this just involves clearing out all the STL containers.
    void        cancel();

    // Support for commit/cancel ops. These are used for
    // e.g. load and unload operations.
    void        add_commit_cancel_op(CdlTransactionCommitCancelOp *);
    void        cancel_last_commit_cancel_op();
    CdlTransactionCommitCancelOp* get_last_commit_cancel_op() const;
    const std::vector<CdlTransactionCommitCancelOp*>& get_commit_cancel_ops() const;
    
    // Propagation support
    void        add_active_change(CdlNode);
    void        add_legal_values_change(CdlValuable);
    void        propagate();
    bool        is_propagation_required() const;
    
    // Inference engine support.
    void        resolve(int = 0); // Process the new conflicts raised by this transaction
    void        resolve(CdlConflict, int = 0);
    void        resolve(const std::vector<CdlConflict>&, int = 0);
    
    // An auxiliary function called by the inference engine to perform recursion
    bool        resolve_recursion(int);
    
    // This function combines propagation, inference, and commit
    // in one easy-to-use package
    void        body();
    
    // Changes.
    // There is a call to get hold of a CdlValue reference. Modifications
    // should happen via a sequence of the form:
    //
    //    valuable->set_value(transact, ...)
    //        const CdlValue& old_value = transact->get_whole_value(CdlValuable);
    //        CdlValue new_value = old_value;
    //        <modify new_value>
    //        transact->set_whole_value(CdlValuable, old_value, new_value);
    //
    // When appropriate the get_whole_value() call takes care of
    // updating the current conflict's solution_references vector. The
    // set_whole_value() call updated the per-transaction changes map,
    // and also stores sufficient information to support propagation.
    // set_whole_value() requires both the old and new values, so
    // that propagation can be optimized.
    const CdlValue&     get_whole_value(CdlConstValuable) const;
    void                set_whole_value(CdlValuable, const CdlValue&, const CdlValue&);

    // Control over active vs. inactive also needs to happen inside
    // transactions
    bool                is_active(CdlNode) const;
    void                set_active(CdlNode, bool);
    
    // Callback and parameter settings
    static void (*get_callback_fn())(const CdlTransactionCallback&);
    static void                 set_callback_fn(void (*)(const CdlTransactionCallback&));
    static void                 set_inference_callback_fn(CdlInferenceCallback);
    static CdlInferenceCallback get_inference_callback_fn();
    static void                 enable_automatic_inference();
    static void                 disable_automatic_inference();
    static bool                 is_automatic_inference_enabled();
    static void                 set_inference_recursion_limit(int);
    static int                  get_inference_recursion_limit();
    // The override indicates the highest level of value source that the
    // library can overwrite without needing user confirmation. The
    // default value is CdlValueSource_Inferred, indicating that the
    // library can overwrite default and inferred values but not
    // wizard or user values.
    static void                 set_inference_override(CdlValueSource);
    static CdlValueSource       get_inference_override();
    
    bool        check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:

  private:

    CdlTransactionBody(CdlToplevel, CdlTransaction, CdlConflict);

    // The associated toplevel and optionally the parent transaction
    // and the conflict being worked on
    CdlToplevel         toplevel;
    CdlTransaction      parent;
    CdlConflict         conflict;
    
    // Per-transaction information. All value changes, new conflicts
    // etc. first live in the context of a transaction. The global
    // configuration only gets updated if the transaction is commited.
    // There is also a vector of the pending commit/cancel ops.
    std::vector<CdlTransactionCommitCancelOp*> commit_cancel_ops;
    std::map<CdlValuable, CdlValue> changes;
    std::list<CdlConflict>          new_conflicts;
    std::list<CdlConflict>          new_structural_conflicts;
    std::vector<CdlConflict>        deleted_conflicts;  // Existing global ones
    std::vector<CdlConflict>        deleted_structural_conflicts;
    std::vector<CdlConflict>        resolved_conflicts; // New ones already fixed by the inference engine
    std::list<CdlConflict>          global_conflicts_with_solutions;
    std::set<CdlNode>               activated;
    std::set<CdlNode>               deactivated;
    std::set<CdlValuable>           legal_values_changes;
    bool                            dirty;
    
    // Change propagation. It is necessary to keep track of all
    // pending value changes, active changes, and of things being
    // loaded or unloaded. The set_value() call is used to update the
    // value_changes container.
    std::deque<CdlValuable>     value_changes;
    std::deque<CdlNode>         active_changes;


    // Control over the inference engine etc.
    static CdlInferenceCallback inference_callback;
    static bool                 inference_enabled;
    static int                  inference_recursion_limit;
    static CdlValueSource       inference_override;
    static void (*callback_fn)(const CdlTransactionCallback&);
    
    enum {
        CdlTransactionBody_Invalid = 0,
        CdlTransactionBody_Magic   = 0x3f91e4df
    } cdltransactionbody_cookie;

    // Illegal operations
    CdlTransactionBody();
    CdlTransactionBody(const CdlTransactionBody &);
    CdlTransactionBody& operator=(const CdlTransactionBody&);
};

//}}}
//{{{  CdlTransactionCallback           

// ----------------------------------------------------------------------------
// The callback class is used to inform applications about all the
// changes that are happening, including side effects. Application
// code can install a callback function which gets invoked at the
// end of every transaction.
//
// NOTE: this implementation is preliminary. In particular it is
// not extensible, it only deals with changes relevant to software
// configurations.

class CdlTransactionCallback {
    
    friend class CdlTest;
    friend class CdlTransactionBody;
    
  public:
    ~CdlTransactionCallback();
    static void (*get_callback_fn())(const CdlTransactionCallback&);
    static void set_callback_fn(void (*)(const CdlTransactionCallback&));

    // Callback functions should be able to retrieve information
    // about the current transaction and toplevel, to avoid the use
    // of statics.
    CdlTransaction              get_transaction() const;
    CdlToplevel                 get_toplevel() const;
    
    // active_changes and legal_values_changes get updated as the
    // transaction proceeds, so a set implementation is more
    // efficient. The others get filled in during a commit operation.
    // A transaction may result in multiple conflicts for a given node
    // being eliminated, so again a set is appropriate. For the others
    // there is no possibility of duplicates so a vector is better.
    std::vector<CdlValuable>    value_changes;
    std::vector<CdlNode>        active_changes;
    std::vector<CdlValuable>    legal_values_changes;
    std::vector<CdlValuable>    value_source_changes;
    std::vector<CdlConflict>    new_conflicts;
    std::vector<CdlConflict>    new_structural_conflicts;
    std::vector<CdlNode>        nodes_with_resolved_conflicts;
    std::vector<CdlNode>        nodes_with_resolved_structural_conflicts;
    
    bool check_this(cyg_assert_class_zeal = cyg_quick) const;
    
  protected:

  private:
    CdlTransactionCallback(CdlTransaction);
    CdlTransaction      transact;

    // Illegal operation.
    CdlTransactionCallback();
    
    enum {
        CdlTransactionCallback_Invalid     = 0,
        CdlTransactionCallback_Magic       = 0x0cec3a95
    } cdltransactioncallback_cookie;
};

//}}}
//{{{  CdlLocalTransaction              

// ----------------------------------------------------------------------------
// A utility class to create a per-function transaction object which gets
// cleaned up automatically should an exception happen.

class CdlLocalTransaction {
    
    friend class CdlTrest;
    
  public:
    CdlLocalTransaction(CdlToplevel toplevel) {
        transaction = CdlTransactionBody::make(toplevel);
    }
    ~CdlLocalTransaction() {
        // The destructor may get invoked during exception handling.
        // It is assumed that cancelling the transaction would be a
        // good thing when that happens. Normal operation should
        // go through the body() or commit() members, which clear
        // the transaction field.
        // There is a slight consistency here. Normally after a
        // transaction commit the transaction object is still
        // around. Here the transaction object get deleted. This
        // is unlikely to matter in practice.
        if (0 != transaction) {
            transaction->cancel();
            delete transaction;
        }
    }
    CdlTransaction get() {
        return transaction;
    }
    void body() {
        transaction->body();
        delete transaction;
        transaction = 0;
    }
    void commit() {
        transaction->commit();
        delete transaction;
        transaction = 0;
    }
    void propagate() {
        transaction->propagate();
    }
    void destroy() {
        if (0 != transaction) {
            transaction->cancel();
            delete transaction;
            transaction = 0;
        }
    }

  private:
    CdlTransaction transaction;
    CdlLocalTransaction();
};

//}}}

//}}}
//{{{  Build and define information                     

//{{{  Description                      

// ----------------------------------------------------------------------------
// There are two related concepts: buildable components, and
// definable components. The former typically refers to compiling
// sources files to produce libraries, although other types of build
// are possible. The latter refers to generating header files
// containing the current configuration data. Typically any loadable
// that is buildable is also definable, so that the source files can
// #include the appropriate generated headers and adapt to the
// configuration data that way. The inverse is not true: for example
// in HCDL it may be appropriate to generate a header file but there
// is nothing to be compiled, device drivers are software packages.
//
// The relevant base classes are as follows:
//
// 1) CdlBuildable      - this object can have build-related properties.
//                        All buildables are also valuables.
// 2) CdlBuildLoadable  - this is a base class for loadables, providing
//                        some extra properties that are relevant for
//                        loadables that can involve builds.
// 3) CdlDefinable      - this object can result in #define's in a
//                        header file. All exportables are also
//                        valuables.
// 4) CdlDefineLoadable - this is a base class for any loadables that
//                        can contain buildables.
//
// Support for both buildable and exportable components is part of the
// core library for now. This may change in future, depending on how
// many CDL variants get implemented.
//
// There are various properties related to building. First, the
// ones applicable to the CdlBuildLoadable class.
//
// 1) library xyz.
//    This specifies the default library for anything built in this
//    loadable. If there is no library property then it defaults to
//    libtarget.a (or rather to a class static that happens to be
//    initialized to libtarget.a)
//
// 2) include_dir <dir>.
//    This specifies where the loadable's exported header files should
//    end up. The default value is the toplevel, but e.g. the eCos
//    kernel specifies an include_dir of cyg/kernel. Note that fixed
//    header files are associated with buildables, not definables,
//    the latter deal with generated header files only.
//
// 3) include_files <hdr1 hdr2 ...>
//    The recommended directory hierarchy for non-trivial packages
//    involves separate subdirectories src, include, cdl, doc, and
//    test. This is too heavyweight for very simple packages where it
//    is better to keep everything in just one directory. However that
//    introduces a potential conflict between public and private
//    header files, which can be resolved by the include_files
//    property. The actual rules are:
//
//    a) if there an include_files property, that lists all the
//       headers that should be exported.
//
//    b) else if there is an include subdirectory, it is assumed that
//       all files below that should be exported.
//
//    c) otherwise all files matching a suitable glob pattern should
//       be exported. The default pattern is *.h *.hxx *.inl, but can
//       be overwritten.
//
// 4) makefile <file>
//    This allows component developers to provide a GNU makefile to be
//    used for building, rather than specify the relevant information
//    via properties.
//    NOTE: this property is ignored for now. It is roughly
//    equivalent to a custom build step where the command is
//    "make -C <dir> -f <file>", but in addition it is necessary to
//    worry about phony targets for default, clean, etc.
//
// A DefineLoadable adds the following property:
//
// 1) define_header <file>
//    This specifies the header file that will be generated. If this
//    property is absent then the library will generate a default one
//    based on the loadable's name, by discarding everything up to and
//    including the first underscore, lowercasing the rest, and
//    appending .h. For example, CYGPKG_KERNEL would result in a
//    header file kernel.h.
//
//    Hardware packages have an implicit "define_header hardware.h"
//    property.
//  
// A buildable has the following properties:
//
// 1) compile [-library xyz] <file1> <file2> ...
//    This specifies one or more files that need to be compiled.
//    By default the resulting object files will go into the
//    current library (set via a higher-level library or
//    defaulting to libtarget.a).
//
//    Legitimate filename suffixes for compile statements are .c, .cxx
//    and .S. Further suffixes may be supported in future. In the
//    long term we will need some external data files defining how
//    the various suffixes should be handled.
//
//    Associated with every compilation are details of the compiler to
//    be used and the compiler flags. For now no attempt is made
//    to do anything interesting in this area, although there is
//    sufficient information in the database for the needs of
//    command line tools.
//
//    Longer term there are complications. Packages may want some
//    control over the compiler flags that should be used, e.g.
//    "requires {!(flags ~= ".*-fno-rtti.*")}" to guarantee that the
//    compiler flags do not include -fno-rtti, rather useful if the
//    package's source code depends on that language feature. Mixed
//    architecture systems (e.g. ARM/Thumb) will cause problems when
//    it comes to selecting the compiler. The exact means by which
//    all this will work is not yet clear.
//
// 2) object [-library xyz] <file1> <file2> ...
//    This specifies one or more pre-built object files that should
//    go into the appropriate library.
//
//    The problem here is coping with different architectures, and for
//    many architectures it will also be necessary to worry about
//    multilibs. Third party component vendors are unlikely to supply
//    separate object files for every supported architecture and every
//    valid multilib within those architectures, so there are
//    constraints on the multilib-related compiler flags used for
//    building other packages and the application itself.
//
//    NOTE: this property is ignored for now.
//
// 3) make_object [-library xyz] [-priority pri] <file> <makefile fragment>
//
//    For example:
//
//    make_object toyslock.o {
//        toyslock.o : toyslock.y
//                yacc toyslock.y
//                $(CC) $(CFLAGS) -o toyslock.o y.tab.c
//    }
//
//    This defines a custom build step for an object file that
//    should go into a particular directory. A makefile syntax
//    is used to define the rule simply because it is likely
//    to be familiar to package developers, and does not
//    imply that the builds will happen via a makefile.
//
//    The optional priority field indicates at which stage during
//    the build the rule should trigger. The default value is
//    100, which is the same as for all files specified in
//    "compile" properties. A lower value means that the object
//    will be generated earlier. Libraries are generated at
//    priority 200, and "make" properties normally execute at
//    priority 300.
//    NOTE: it is not clear yet whether supporting priorities
//    in this way is a good idea, or whether the dependencies
//    information could be used instead.
//
//    Unresolved issues:
//
//    a) what commands can be used in the build rules? There
//       should be a core set of supported commands, as per
//       an eCos toolchain build. It should also be possible
//       for packages to provide their own host tools.
//
//       For sourceware folks, moving away from a single toolchain
//       tarball and expecting them to download and install
//       egcs, binutils and gdb separately is actually a bad
//       idea in this regard, it makes it much more likely that
//       some users will have an incomplete tools installation and
//       hence that builds will fail.
//
//    b) there is an obvious need for variable substitution in the
//       rules, e.g. $(CC). At what stage do these variables get
//       expanded, and where does the required information live?
//
//    c) who is responsible for header file dependency analysis?
//       Should the rules be modified automatically to do this,
//       or do we leave this to the package developer? It may be
//       very hard to do the former, but the latter will cause
//       problems for IDE integration.
//
//    d) in which directory will the rules get run? What prevents
//       filename conflicts between different packages?
//
//    NOTE: make_object is not actually required just yet, but the
//    issues are much the same as for the "make" property which is
//    required.
//
// 4) make [-priority pri] <target> <makefile fragment>
//
//    For example:
//
//    make target.ld {
//    target.ld : arm.ld
//            $(CC) -E -P -xc $(CFLAGS) -o $@ $<
//    }
//
//    This defines a custom build step for a target that is not going
//    to end up in a library. The main such targets at the moment are
//    the linker script, vectors.o, and extras.o, but there may well
//    be others in future.
//
//    The default priority for "make" properties is 300, which means
//    that the build rules trigger after all normal compilations and
//    after the libraries are generated. It is possible to specify
//    custom build steps that should run before any compilations
//    using a priority < 100.
//
//    Unresolved issues:
//
//    a) what commands can be used?
//
//    b) variable substitution?
//
//    c) header file dependency analysis?
//
//    d) directories and filenames?
//
//    e) where should the resulting files end up? Currently they can
//       all go into $(PREFIX)/lib, but in the long term we may
//       need to be a bit more flexible.
//
// 5) build_proc <tcl code>
//
//    This defines some Tcl code that should be run prior to any
//    build, for example to generate a source file. It must run
//    within the appropriate loadable's Tcl interpreter so that
//    it can query the current configuration.
//
//    NOTE: this property is not implemented yet.
//
//
// A definable has the following properties:
//
// 1) no_define
//    Usually the library will generate either one or two #define's
//    for every definable, inside the current header file. This can be
//    suppressed by the no_define property, which is typically
//    accompanied by some other #define-related property such as
//    define_proc or define.
//
// 2) define [-file <filename>] [-format <format_string>] symbol
//    This will result in an additional #define for the specified
//    symbol in the specified file. The only filenames that are valid
//    are the loadable's current filename (as per define_header), and
//    the global header file system.h. Use of the latter should be
//    avoided.
//
//    The optional format string behaves as per the define_format
//    property below.
//
// 3) define_format <format_string>
//    This is only relevant for booldata or data flavors. By default
//    two #define's will be generated (assuming the valuable is active
//    and enabled):
//
//        #define <symbol> value
//        #define <symbol>_value
//
//    The latter will only be generated if the resulting symbol is
//    a valid C preprocessor symbol, and is intended to allow the
//    use of #ifdef as well as #ifdef (useful if the value is
//    non-numerical). 
//
//    The define_format property provides control over the first of
//    these two #defines. The net result is that the #define will be
//    generated by evaluating the following Tcl fragment:
//
//        set result "#define <symbol> [<format> <value>]"
//
//    Command and variable substitution are available if desired,
//    but for anything that complicated the define_proc property
//    is normally more useful.
//
//    define_format is only applicable to the default definition,
//    so it cannot be used in conjunction with no_define. The
//    define property supports a -format option.
//
// 4) define_proc <tclcode>
//    This specifies some Tcl code that should be run when header
//    file generation takes place, in addition to any #define's
//    generated by default or courtesy of define properties.
//    The define_proc property is commonly used in conjunction with
//    no_define, but this is not required.
//
//    There will be two channels already set up: cdl_header
//    for the current loadable, and cdl_system_header for system.h.
//    Writing data to system.h should be avoided.
//
// 5) if_define <condition> <symbol>

//    This property provides direct support for a common programming
//    paradigm. It allows direct generation of code like the
//    following:
//
//    #ifdef CYGSRC_TOYS_BLOCKS
//    # define CYGDBG_INFRA_USE_PRECONDITIONS 1
//    #endif
//
//    In this case CYGSRC_TOYS_BLOCKS is the condition and
//    CYGDBG_INFRA_USE_PRECONDITIONS is the symbol. The
//    #ifdef/#define sequence will be generated in addition to
//    any other #define's resulting from the default behaviour,
//    the define property, or the define_proc property. It is
//    not affected by no_define.

//}}}
//{{{  The build process                

// ----------------------------------------------------------------------------
// For command-line operation the steps involved in doing a build are:
//
// 1) work out what needs to be built.
//
// 2) generate a build and install tree. This involves making sure that
//    the various directories exist and are accessible.
//
// 3) generate or update the toplevel makefile.
//
// 4) generate the configuration header files.
//
// For operation in an IDE steps (2) and (3) will be handled by
// different code.
//
// There is a library call to get hold of all the build information:
//
//     config->get_build_info(CdlBuildInfo &info);
//
// This erases anything previously present in the build-info argument
// and fills in the information appropriate to the current
// configuration, essentially by walking down the list of loadables
// and each loadable's list of nodes, checking for BuildLoadables
// and Buildables along the way. The BuildInfo class is defined
// further down.
//
// An alternative library call can be used to find out about all
// possible files that need to be compiled etc., irrespective of the
// current configuration settings. This could be useful when it
// comes to letting the user control compiler flags etc.
//
//    config->get_all_build_info(CdlBuildInfo& info);
//
// There is another library call for step (4):
//
//    config->generate_config_headers(std::string directory)
//
// This will create or update the header files appropriate to
// the current configuration. Temporary files will be generated,
// diff'ed with the current version, and existing files will
// only be modified if necessary. The directory argument
// indicates where the header files should go, i.e. it should
// be the equivalent of $(PREFIX)/include/pkgconf
//
// This library call does not delete any files it does not
// recognize, that is the responsibility of higher-level code.
// It is possible to get or update a list of the files that
// will be generated:
//
//    config->get_config_headers(std::vector<std::string>& headers)
//
// The argument will be cleared if necessary and then filled in with
// the current set of header files. Higher level code can compare the
// result with the current files in the directory and take or suggest
// remedial action.
//
// There is also a library call which combines all four stages:
//
//    config->generate_build_tree(std::string build_tree, std::string prefix = $(BUILD)/install)
//
//
// The order in which the various build steps happen is important.
//
// 1) non-configuration headers must be copied from the component
//    repository into $(PREFIX)/include. No compiles can happen
//    before this.
//
// 2) all compile properties can happen in parallel. These have an
//    effective priority of 100.
//
// 3) all make_object priorities can happen in parallel with
//    compiles. These have a default priority of 100, but the
//    priority can be modified.
//
// 4) the generated objects and any pre-built objects should be
//    incorporated into the appropriate library. This happens
//    at priority 200.
//
// 5) custom build steps associated with "make" properties should
//    now run. These have a default priority of 300, but it is
//    possible to override this.
//
// Usually all source files will come from the component repository,
// which means that they are read-only. Ideally it should also be
// possible for a source file to be copied into the build tree and
// edited there, and subsequent builds should pick up the copy rather
// than the original. The build data generated by libcdl will always
// be in the form of relative pathnames to facilitate this.

//}}}
//{{{  CdlBuildInfo class               

// ----------------------------------------------------------------------------
// Extracting the build information.
//
// libcdl.a defines the following classes related to build information.
//
// CdlBuildInfo
// CdlBuildInfo_Loadable
// CdlBuildInfo_Header
// CdlBuildInfo_Compile
// CdlBuildInfo_Object
// CdlBuildInfo_MakeObject
// CdlBuildInfo_Make
//
// The build information is organized on a per-loadable basis.
// Higher-level code may choose to flatten this or to keep the
// distinction. A CdlBuildInfo object is primarily a vector of
// CdlBuildInfo_Loadable objects. CdlBuildInfo objects can be created
// statically.
//
// In turn, each CdlBuildInfo_Loadable object is primarily a
// collection of five vectors, one each for Header, Compile, Object,
// MakeObject and Make.
//
// All pathnames in these data structures will use forward slashes as
// the directory separator, irrespective of the host platform. All
// pathnames will be relative.

struct CdlBuildInfo_Header {
    std::string         source;         /* include/cyg_ass.h    */
    std::string         destination;    /* cyg/infra/cyg_ass.h  */
};

struct CdlBuildInfo_Compile {
    std::string         library;        /* libtarget.a          */
    std::string         source;         /* src/fancy.cxx        */
    // Compiler and cflags data may be added in future.
};

struct CdlBuildInfo_Object {
    std::string         library;        /* libtarget.a          */
    std::string         object;         /* obj/hello.o          */
};

struct CdlBuildInfo_MakeObject {
    cdl_int             priority;       /* 100                  */
    std::string         library;        /* libtarget.a          */
    std::string         object;         /* toyslock.o           */
    std::string         deps;           /* toyslock.y           */
    /*
      It is not clear whether the deps field is actually useful in the
      context of IDE integration, but see the note about arm.inc
      above.
    */
    std::string         rules;
    /*
      A typical value for "rules" might be:

        yacc toyslock.y
        $(CC) $(CFLAGS) -o toyslock.o y.tab.c
        
      Leading white space is not significant. Newlines are significant.
      Backslash escapes in the text will not have been processed yet.
    */  
};

struct CdlBuildInfo_Make {
    cdl_int             priority;       /* 300                  */
    std::string         target;         /* extras.o             */
    std::string         deps;           /* libextras.a          */
    std::string         rules;
    /*
      Something like:
      
  $(CC) $(ARCHFLAGS) $(LDARCHFLAGS) -nostdlib -Wl,-r -Wl,--whole-archive $(PREFIX)/lib/libextras.a -o $(PREFIX)/lib/extras.o
  
    */
};

class CdlBuildInfo_Loadable {
    
    friend class CdlTest;

  public:
    std::string         name;           /* CYGPKG_INFRA         */
    std::string         directory;      /* infra/current        */
    std::vector<CdlBuildInfo_Header>            headers;
    std::vector<CdlBuildInfo_Compile>           compiles;
    std::vector<CdlBuildInfo_Object>            objects;
    std::vector<CdlBuildInfo_MakeObject>        make_objects;
    std::vector<CdlBuildInfo_Make>              makes;
    
  protected:

  private:
};

class CdlBuildInfo {

    friend class CdlTest;

  public:

    std::vector<CdlBuildInfo_Loadable>  entries;

  protected:

  private:
};

//}}}
//{{{  CdlBuildLoadable                 

// ----------------------------------------------------------------------------
// BuildLoadables are derived from Loadables and are appropriate for
// any loadables that can contain build information. There are a
// number of properties applicable at this level: makefile,
// include_dir, include_files and library. The main interface of
// interest is update_build_info().
//
// It is likely that all BuildLoadables are also Buildables, but this
// is not required.

class CdlBuildLoadableBody : virtual public CdlLoadableBody
{
    friend class CdlTest;

  public:
    virtual ~CdlBuildLoadableBody();

    // This is the main way to extract information about what should
    // get built. It takes into account the active and enabled states,
    // as appropriate.
    void        update_build_info(CdlBuildInfo&) const;

    // An alternative which ignores the active and enabled states.
    void        update_all_build_info(CdlBuildInfo&) const;
    
    // Property parsers and validation code appropriate for a
    // build-loadable object such as makefile
    static void add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers);
    void        check_properties(CdlInterpreter);
    static int  parse_library(CdlInterpreter, int, const char*[]);
    static int  parse_makefile(CdlInterpreter, int, const char*[]);
    static int  parse_include_dir(CdlInterpreter, int, const char*[]);
    static int  parse_include_files(CdlInterpreter, int, const char*[]);
    
    // By default any compiled files will go into libtarget.a, which
    // is the default value for this variable. Individual applications may
    // specify an alternative default library.
    static char*        default_library_name;

    // When filling in a build_info structure the library needs to know
    // what constitutes a header file. A glob pattern can be used for this.
    // NOTE: in the long term this should come out of a data file.
    static char*        default_headers_glob_pattern;
    
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:
    CdlBuildLoadableBody();

  private:

    enum {
        CdlBuildLoadableBody_Invalid    = 0,
        CdlBuildLoadableBody_Magic      = 0x55776643
    } cdlbuildloadablebody_cookie;

    // Illegal operations
    CdlBuildLoadableBody(const CdlBuildLoadableBody&);
    CdlBuildLoadableBody& operator=(const CdlBuildLoadableBody&);
};

//}}}
//{{{  CdlBuildable                     

// ----------------------------------------------------------------------------
// Buildable objects can have properties such as compile and
// make_object. These properties are not normally accessed
// directly. Instead there is a member function to update a
// CdlBuildInfo_Loadable object.
//
// The build properties for a given buildable have an effect iff
// that buildable is active, and in addition if the buildable is also
// a valuable then it must be enabled.

class CdlBuildableBody : virtual public CdlNodeBody
{

    friend class CdlTest;

  public:
    virtual ~CdlBuildableBody();

    // This is the main way to extract information about what should
    // get built. It takes into account the active and enabled states,
    // as appropriate. The second argument indicates the default
    // library for the current loadable.
    void        update_build_info(CdlBuildInfo_Loadable&, std::string) const;

    // An alternative which ignores the active and enabled states.
    void        update_all_build_info(CdlBuildInfo_Loadable&, std::string) const;
    
    // Add property parsers and validation code appropriate for a
    // buildable object such as compile and make_object
    static void add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers);
    void        check_properties(CdlInterpreter);

    static int  parse_build_proc(CdlInterpreter, int, const char*[]);
    static int  parse_compile(CdlInterpreter, int, const char*[]);
    static int  parse_make(CdlInterpreter, int, const char*[]);
    static int  parse_make_object(CdlInterpreter, int, const char*[]);
    static int  parse_object(CdlInterpreter, int, const char*[]);
    static bool split_custom_build_step(std::string /* data */, std::string& /* target */, std::string& /* deps */,
                                        std::string& /* rules*/, std::string& /* error_msg */);

    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:
    CdlBuildableBody();

  private:

    enum {
        CdlBuildableBody_Invalid        = 0,
        CdlBuildableBody_Magic          = 0x16eb1c04
    } cdlbuildablebody_cookie;

    // Illegal operations
    CdlBuildableBody(const CdlBuildableBody&);
    CdlBuildableBody& operator=(const CdlBuildableBody&);
};

//}}}
//{{{  CdlDefineLoadable                

// ----------------------------------------------------------------------------
// DefineLoadables are derived from Loadables and are appropriate for
// any loadables that can result in generated header files containing
// configuration data. There is one applicable property,
// define_header. The main interface of interest is
// generate_config_headers().

class CdlDefineLoadableBody : virtual public CdlLoadableBody
{

    friend class CdlTest;

  public:
    virtual ~CdlDefineLoadableBody();

    // Update the header file for this loadable. The first argument
    // is a channel to the loadable-specific header file. The second
    // argument is a channel to the global header file.
    void        generate_config_header(Tcl_Channel, Tcl_Channel) const;

    // What header file should be generated for this loadable?
    virtual std::string get_config_header() const;
    
    // Add property parsers and validation code.
    static void         add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers);
    void                check_properties(CdlInterpreter);
    static int          parse_define_header(CdlInterpreter, int, const char*[]);

    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:
    CdlDefineLoadableBody();

  private:

    enum {
        CdlDefineLoadableBody_Invalid   = 0,
        CdlDefineLoadableBody_Magic     = 0x7e211709
    } cdldefineloadablebody_cookie;

    // Illegal operations
    CdlDefineLoadableBody(const CdlDefineLoadableBody&);
    CdlDefineLoadableBody& operator=(const CdlDefineLoadableBody&);
};

//}}}
//{{{  CdlDefinable                     

// ----------------------------------------------------------------------------
// Definables are derived from Valuables and provide support for
// outputting a configuration header file.

class CdlDefinableBody : virtual public CdlValuableBody
{

    friend class CdlTest;

  public:
    virtual ~CdlDefinableBody();

    // Update the header file for this definable. The loadable's Tcl
    // interpreter will already have channels cdl_header and
    // cdl_system_header set up appropriately.
    void        generate_config_header( Tcl_Channel, Tcl_Channel) const;

    // Add property parsers and validation code.
    static void add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers);
    void        check_properties(CdlInterpreter);
    static int  parse_define(CdlInterpreter, int, const char*[]);
    static int  parse_define_format(CdlInterpreter, int, const char*[]);
    static int  parse_define_proc(CdlInterpreter, int, const char*[]);
    static int  parse_if_define(CdlInterpreter, int, const char*[]);
    static int  parse_no_define(CdlInterpreter, int, const char*[]);

    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();
    
  protected:
    CdlDefinableBody();

  private:

    enum {
        CdlDefinableBody_Invalid        = 0,
        CdlDefinableBody_Magic          = 0x65a2c95a
    } cdldefinablebody_cookie;

    // Illegal operations
    CdlDefinableBody(const CdlDefinableBody&);
    CdlDefinableBody& operator=(const CdlDefinableBody&);
};

//}}}

//}}}
//{{{  CdlDialog                                        

// ----------------------------------------------------------------------------
// A dialog simply inherits from CdlUserVisible and provides convenient
// access to several dialog-specific properties.

class CdlDialogBody :
    public virtual CdlUserVisibleBody,
    public virtual CdlParentableBody
{
    friend class CdlTest;

  public:

    virtual ~CdlDialogBody();

    // Dialogs may be enabled or disabled globally. This affects
    // CdlValuable::get_widget_hint() if the valuable has an associated
    // custom dialog.
    static void         disable_dialogs();
    static void         enable_dialogs();
    static bool         dialogs_are_enabled();

    bool                has_init_proc() const;
    bool                has_update_proc() const;
    const cdl_tcl_code& get_init_proc() const;
    const cdl_tcl_code& get_update_proc() const;
    const cdl_tcl_code& get_display_proc() const;
    const cdl_tcl_code& get_confirm_proc() const;
    const cdl_tcl_code& get_cancel_proc() const;
    
    static int          parse_dialog(CdlInterpreter, int, const char*[]);
    static int          parse_display_proc(CdlInterpreter, int, const char*[]);
    static int          parse_update_proc(CdlInterpreter, int, const char*[]);
    
    // Persistence support. Dialogs should just be ignored when it
    // comes to saving and restoring files.
    virtual void        save(CdlInterpreter, Tcl_Channel, int, bool);
                                                
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  private:
    // The constructor only gets invoked from inside parse_dialog()
    CdlDialogBody(std::string);

    static bool         dialogs_enabled;
    
    enum {
        CdlDialogBody_Invalid   = 0,
        CdlDialogBody_Magic     = 0x3f4df391
    } cdldialogbody_cookie;
    
    // Illegal operations. The dialog name must be known at the time
    // that the object is constructed.
    CdlDialogBody();
    CdlDialogBody(const CdlDialogBody&);
    CdlDialogBody& operator=(const CdlDialogBody&);
};

//}}}
//{{{  CdlWizard                                        

// ----------------------------------------------------------------------------
// A wizard is very much like a dialog, just a different set of properties.

class CdlWizardBody :
    public virtual CdlUserVisibleBody,
    public virtual CdlParentableBody
{
    friend class CdlTest;

  public:

    virtual ~CdlWizardBody();

    bool                has_init_proc() const;
    bool                has_decoration_proc() const;
    const cdl_tcl_code& get_init_proc() const;
    const cdl_tcl_code& get_decoration_proc() const;
    const cdl_tcl_code& get_confirm_proc() const;
    const cdl_tcl_code& get_cancel_proc() const;
    bool                has_screen(cdl_int) const;
    cdl_int             get_first_screen_number() const;
    const cdl_tcl_code& get_first_screen() const;
    const cdl_tcl_code& get_screen(cdl_int) const;
    static int          parse_wizard(CdlInterpreter, int, const char*[]);
    static int          parse_cancel_proc(CdlInterpreter, int, const char*[]);
    static int          parse_confirm_proc(CdlInterpreter, int, const char*[]);
    static int          parse_decoration_proc(CdlInterpreter, int, const char*[]);
    static int          parse_init_proc(CdlInterpreter, int, const char*[]);
    static int          parse_screen(CdlInterpreter, int, const char*[]);
    
    // Persistence support. Wizards should just be ignored when it
    // comes to saving and restoring files.
    virtual void        save(CdlInterpreter, Tcl_Channel, int, bool);
    
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  private:
    // The constructor only gets invoked from inside parse_wizard().
    CdlWizardBody(std::string);

    // Illegal operations.
    CdlWizardBody();
    CdlWizardBody(const CdlWizardBody&);
    CdlWizardBody& operator=(const CdlWizardBody&);
    
    enum {
        CdlWizardBody_Invalid   = 0,
        CdlWizardBody_Magic     = 0x4ec1c39a
    } cdlwizardbody_cookie;
};

//}}}
//{{{  CdlInterface class                               

// ----------------------------------------------------------------------------
// Similarly for interfaces.

class CdlInterfaceBody : public virtual CdlNodeBody,
                         public virtual CdlUserVisibleBody,
                         public virtual CdlValuableBody,
                         public virtual CdlParentableBody,
                         public virtual CdlBuildableBody,
                         public virtual CdlDefinableBody
{
    friend class CdlTest;

  public:

    ~CdlInterfaceBody();

    void                get_implementers(std::vector<CdlValuable>&) const;
    void                recalculate(CdlTransaction);
    
    static int          parse_interface(CdlInterpreter, int, const char*[]);
    
    // Persistence support. The interface data cannot sensibly be modified
    // by users, it is all calculated. However it is useful to have the
    // interface data present in the saved file so that users can examine
    // dependencies etc.
    virtual void        save(CdlInterpreter, Tcl_Channel, int, bool);
    static void         initialize_savefile_support(CdlToplevel);
    static int          savefile_interface_command(CdlInterpreter, int, const char*[]);

    bool                was_generated() const;
    virtual bool        is_modifiable() const;
    virtual std::string get_class_name() const;
    bool                check_this(cyg_assert_class_zeal = cyg_quick) const;
    CYGDBG_DECLARE_MEMLEAK_COUNTER();

  private:
    CdlInterfaceBody(std::string, bool /* generated */);
    bool        generated;
    
    enum {
        CdlInterfaceBody_Invalid   = 0,
        CdlInterfaceBody_Magic     = 0x67f7fbe5
    } cdlinterfacebody_cookie;
    CdlInterfaceBody();
    CdlInterfaceBody(const CdlInterfaceBody&);
    CdlInterfaceBody& operator=(const CdlInterfaceBody&);
};

//}}}

#endif  /* !__CDLCORE_HXX */
// EOF cdlcore.hxx
