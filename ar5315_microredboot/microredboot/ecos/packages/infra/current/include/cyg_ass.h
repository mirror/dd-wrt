#ifndef CYGONCE_INFRA_CYG_ASS_H
#define CYGONCE_INFRA_CYG_ASS_H

//==========================================================================
//
//      assert.h
//
//      Macros and prototypes for the assert system
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg from an original by hmt
// Contributors:        nickg
// Date:        1997-09-08
// Purpose:     Use asserts to avoid writing duff code.
// Description: Runtime tests that compile to nothing in
//              release versions of the code, to allow
//              as-you-go testing of alternate builds.
// Usage:       #include <cyg/infra/cyg_ass.h>
//              ...
//              CYG_ASSERT( pcount > 0, "Number of probes should be > 0!" );
//
//      which can result, for example, in a message of the form:
//      ASSERT FAILED: probemgr.cxx:1340, scan_probes() :
//                     number of probes should be > 0!
//      if the boolean "pcount > 0" is false.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/infra.h>

#include <cyg/infra/cyg_type.h>         // for CYGBLD_ATTRIB_NORET

// -------------------------------------------------------------------------
// If we do not have a function name macro, define it ourselves

#ifndef CYGDBG_INFRA_DEBUG_FUNCTION_PSEUDOMACRO
                                        // __PRETTY_FUNCTION__ does not work
# ifndef __PRETTY_FUNCTION__            // And it is not already defined
#  define __PRETTY_FUNCTION__ NULL
# endif
#endif

// -------------------------------------------------------------------------
// This is executed to deal with failure - breakpoint it first!
// It is declared as a weak symbol to allow user code to override the
// definition.

externC void
cyg_assert_fail( const char* /* psz_func */, const char* /* psz_file */,
                 cyg_uint32 /* linenum */, const char* /* psz_msg */ )  __THROW
    CYGBLD_ATTRIB_NORET CYGBLD_ATTRIB_WEAK;

externC void
cyg_assert_msg( const char *psz_func, const char *psz_file,
                cyg_uint32 linenum, const char *psz_msg ) __THROW;

// -------------------------------------------------------------------------

#ifdef CYGDBG_USE_ASSERTS

// -------------------------------------------------------------------------
// We define macros and appropriate prototypes for the assert/fail
// system.  These are:
//      CYG_FAIL        - unconditional panic
//      CYG_ASSERT      - panic if boolean expression is false
//      CYG_ASSERTC     - compact version of CYG_ASSERT

# ifdef CYGDBG_INFRA_DEBUG_ASSERT_MESSAGE
#  define CYG_ASSERT_DOCALL( _msg_ )                                      \
        CYG_MACRO_START                                                   \
        /* Make sure we always get a pretty-printed message */            \
        cyg_assert_msg( __PRETTY_FUNCTION__, __FILE__, __LINE__, _msg_ ); \
        cyg_assert_fail( __PRETTY_FUNCTION__, __FILE__, __LINE__, _msg_ );\
        CYG_MACRO_END
# else
#   define CYG_ASSERT_DOCALL( _msg_ )    \
        CYG_MACRO_START                 \
        const char* _tmp1_ = _msg_;     \
        _tmp1_ = _tmp1_;                \
        cyg_assert_fail( __PRETTY_FUNCTION__, __FILE__, __LINE__, NULL ); \
        CYG_MACRO_END
# endif

// unconditional failure; use like panic(), coredump() &c.
# define CYG_FAIL( _msg_ )              \
        CYG_MACRO_START                 \
        CYG_ASSERT_DOCALL( _msg_ );      \
        CYG_MACRO_END

// conditioned assert; if the condition is false, fail.
# define CYG_ASSERT( _bool_, _msg_ )    \
        CYG_MACRO_START                 \
        if ( ! ( _bool_ ) )             \
            CYG_ASSERT_DOCALL( _msg_ );  \
        CYG_MACRO_END

# define CYG_ASSERTC( _bool_ )          \
       CYG_MACRO_START                  \
       if ( ! ( _bool_ ) )              \
           CYG_ASSERT_DOCALL( #_bool_ );\
       CYG_MACRO_END

#else // ! CYGDBG_USE_ASSERTS

// -------------------------------------------------------------------------
// No asserts: we define empty statements for assert & fail.

# define CYG_FAIL( _msg_ )           CYG_EMPTY_STATEMENT
# define CYG_ASSERT( _bool_, _msg_ ) CYG_EMPTY_STATEMENT
# define CYG_ASSERTC( _bool_ )       CYG_EMPTY_STATEMENT

#endif // ! CYGDBG_USE_ASSERTS

// -------------------------------------------------------------------------
// Pointer integrity checks.
// These check not only for NULL pointer, but can also check for pointers
// that are outside to defined memory areas of the platform or executable.
// We differentiate between data and function pointers, so that we can cope
// with different formats, and so we can check them against different memory
// regions.

externC cyg_bool cyg_check_data_ptr(const void *ptr);
externC cyg_bool cyg_check_func_ptr(const void (*ptr)(void));

#ifdef CYGDBG_USE_ASSERTS

# define CYG_CHECK_DATA_PTR( _ptr_, _msg_ )             \
        CYG_MACRO_START                                 \
        if( !cyg_check_data_ptr((const void *)(_ptr_)))       \
           CYG_ASSERT_DOCALL( _msg_ );                   \
        CYG_MACRO_END

# define CYG_CHECK_FUNC_PTR( _ptr_, _msg_ )             \
        CYG_MACRO_START                                 \
        if( !cyg_check_func_ptr((const void (*)(void))(_ptr_))) \
           CYG_ASSERT_DOCALL( _msg_ );                   \
        CYG_MACRO_END
        
# define CYG_CHECK_DATA_PTRC( _ptr_ )                   \
         CYG_MACRO_START                                \
         if ( !cyg_check_data_ptr((const void *)(_ptr_)))     \
             CYG_ASSERT_DOCALL("data pointer (" #_ptr_ ") is valid");\
         CYG_MACRO_END

# define CYG_CHECK_FUNC_PTRC( _ptr_ )                       \
         CYG_MACRO_START                                    \
         if ( !cyg_check_func_ptr((const void (*)(void))(_ptr_))) \
             CYG_ASSERT_DOCALL("function pointer (" #_ptr_ ") is valid"); \
         CYG_MACRO_END

#else // CYGDBG_USE_ASSERTS

# define CYG_CHECK_DATA_PTR( _ptr_, _msg_ ) CYG_EMPTY_STATEMENT
# define CYG_CHECK_FUNC_PTR( _ptr_, _msg_ ) CYG_EMPTY_STATEMENT
# define CYG_CHECK_DATA_PTRC( _ptr_ )       CYG_EMPTY_STATEMENT
# define CYG_CHECK_FUNC_PTRC( _ptr_ )       CYG_EMPTY_STATEMENT

#endif // CYGDBG_USE_ASSERTS
            
// -------------------------------------------------------------------------
// Unconditional definitions:

// Check an object for validity by calling its own checker.
// Usage:
//   ClassThing *p = &classobject;
//   CYG_ASSERTCLASS( p, "Object at p is broken!" );

// this enum gives some options as to how keenly to test; avoids cluttering
// the member function declaration if the implementor wants to do more
// zealous tests themselves.

enum cyg_assert_class_zeal {
  cyg_system_test       = -1,
  cyg_none              = 0,
  cyg_trivial,
  cyg_quick,
  cyg_thorough,
  cyg_extreme
};

// -------------------------------------------------------------------------
// Define macros for checking classes:
//
//      CYG_ASSERT_CLASS        - do proforma check on a class pointer
//      CYG_ASSERT_CLASSO       - do proforma check on a class object
//      CYG_ASSERT_ZERO_OR_CLASS- a class pointer is NULL or valid
//      CYG_ASSERT_THIS         - "this" is valid
//      + 3 compact variants and two aliases for backwards compatibility.
//
// All of these end up going via CYG_ASSERT(), which will be an empty
// statement if CYGDBG_USE_ASSERTS is disabled. There is no need to
// test CYGDBG_USE_ASSERTS again here.
//
// The idiom required is that a class have a member function called
// "bool check_this( cyg_assert_class_zeal ) const" that returns true
// iff the object is OK.  This need not be conditionally compiled against
// CYGDBG_USE_ASSERTS but it can be if only this macro is used to
// invoke it.  Alternatively it can be invoked by hand with other
// choices from the above enum.

// Assert the checker function of an object by pointer, or in hand.

#ifdef __cplusplus

# ifndef CYG_ASSERT_CLASS_ZEAL
#  define CYG_ASSERT_CLASS_ZEAL (cyg_quick) // can be redefined locally
# endif

# define CYG_ASSERT_CLASS( _pobj_, _msg_ ) \
    CYG_ASSERT( ((0 != (_pobj_)) &&        \
                 (_pobj_)->check_this( CYG_ASSERT_CLASS_ZEAL )), _msg_ )

# define CYG_ASSERTCLASS( _pobj_,_msg_) \
    CYG_ASSERT_CLASS( (_pobj_), _msg_ )

# define CYG_ASSERT_CLASSO( _obj_, _msg_ ) \
    CYG_ASSERT( (_obj_).check_this( CYG_ASSERT_CLASS_ZEAL ), _msg_ )

# define CYG_ASSERTCLASSO( _obj_, _msg_ ) \
    CYG_ASSERT_CLASSO( (_obj_), _msg_ )

# define CYG_ASSERT_ZERO_OR_CLASS( _pobj_, _msg_ ) \
    CYG_ASSERT( ((0 == (_pobj_)) ||                \
                 (_pobj_)->check_this( CYG_ASSERT_CLASS_ZEAL )), _msg_ )

# define CYG_ASSERT_THIS( _msg_ ) \
    CYG_ASSERT( this->check_this( CYG_ASSERT_CLASS_ZEAL ), _msg_ )

# define CYG_ASSERT_CLASSC( _pobj_ ) \
    CYG_ASSERT_CLASS( (_pobj_), "class pointer (" #_pobj_ ") is valid" )

# define CYG_ASSERT_CLASSOC( _obj_ ) \
    CYG_ASSERT_CLASSO( (_obj_), "object (" #_obj_ ") is valid" )

# define CYG_ASSERT_ZERO_OR_CLASSC( _pobj_ ) \
    CYG_ASSERT_ZERO_OR_CLASS((_pobj_),       \
        "class pointer (" #_pobj_ ") is zero or valid")

# define CYG_ASSERT_THISC( ) \
    CYG_ASSERT_THIS( "\"this\" pointer is valid" )
    
#define CYGDBG_DEFINE_CHECK_THIS \
    cyg_bool check_this( cyg_assert_class_zeal zeal ) const;

#endif // __cplusplus

// -------------------------------------------------------------------------
// Some alternative names for basic assertions that we can disable
// individually.
//
//      CYG_PRECONDITION        - argument checking etc
//      CYG_POSTCONDITION       - results etc
//      CYG_LOOP_INVARIANT      - for putting in loops
//
// C++ programmers have class-related variants of all of these.

#ifdef CYGDBG_INFRA_DEBUG_PRECONDITIONS
# define CYG_PRECONDITION( _bool_ , _msg_ ) CYG_ASSERT( _bool_, _msg_ )
# define CYG_PRECONDITIONC( _bool_ ) \
    CYG_ASSERT( _bool_, "precondition " #_bool_)
#else
# define CYG_PRECONDITION( _bool_ , _msg_ ) CYG_EMPTY_STATEMENT
# define CYG_PRECONDITIONC( _bool_ )        CYG_EMPTY_STATEMENT
#endif

#ifdef CYGDBG_INFRA_DEBUG_POSTCONDITIONS
# define CYG_POSTCONDITION( _bool_ , _msg_ ) CYG_ASSERT( _bool_, _msg_ )
# define CYG_POSTCONDITIONC( _bool_ ) \
    CYG_ASSERT( _bool_, "postcondition " #_bool_)
#else
# define CYG_POSTCONDITION( _bool_ , _msg_ ) CYG_EMPTY_STATEMENT
# define CYG_POSTCONDITIONC( _bool_ )        CYG_EMPTY_STATEMENT
#endif

#ifdef CYGDBG_INFRA_DEBUG_LOOP_INVARIANTS
# define CYG_LOOP_INVARIANT( _bool_ , _msg_ ) CYG_ASSERT( _bool_, _msg_ )
# define CYG_LOOP_INVARIANTC( _bool_ ) \
    CYG_ASSERT( _bool_, "loop invariant " #_bool_ )
#else
# define CYG_LOOP_INVARIANT( _bool_ , _msg_ ) CYG_EMPTY_STATEMENT
# define CYG_LOOP_INVARIANTC( _bool_ )        CYG_EMPTY_STATEMENT
#endif

#ifdef __cplusplus

// All variants of _CLASS
# define CYG_PRECONDITION_CLASS( _pobj_, _msg_ )  \
    CYG_PRECONDITION( ((0 != (_pobj_)) &&         \
                       (_pobj_)->check_this(CYG_ASSERT_CLASS_ZEAL)), _msg_)

# define CYG_PRECONDITION_CLASSC( _pobj_ )        \
    CYG_PRECONDITION_CLASS( (_pobj_),             \
       "precondition, class pointer (" #_pobj_ ") is valid" )
    
# define CYG_POSTCONDITION_CLASS( _pobj_, _msg_ ) \
    CYG_POSTCONDITION( ((0 != (_pobj_)) &&        \
                        (_pobj_)->check_this(CYG_ASSERT_CLASS_ZEAL)), _msg_)

# define CYG_POSTCONDITION_CLASSC( _pobj_ )       \
    CYG_POSTCONDITION_CLASS( (_pobj_),            \
       "postcondition, class pointer (" #_pobj_ ") is valid" )

# define CYG_LOOP_INVARIANT_CLASS( _pobj_, _msg_) \
    CYG_LOOP_INVARIANT( ((0 != (_pobj_)) &&       \
                         (_pobj_)->check_this(CYG_ASSERT_CLASS_ZEAL)), _msg_)
        
# define CYG_LOOP_INVARIANT_CLASSC( _pobj_ )      \
    CYG_LOOP_INVARIANT_CLASS( (_pobj_),           \
       "loop invariant, class pointer (" #_pobj_ ") is valid" )

// All variants of _CLASSO
# define CYG_PRECONDITION_CLASSO( _obj_, _msg_ )  \
    CYG_PRECONDITION( (_obj_).check_this(CYG_ASSERT_CLASS_ZEAL), _msg_)
    
# define CYG_PRECONDITION_CLASSOC( _obj_ )        \
    CYG_PRECONDITION_CLASSO( (_obj_),             \
        "precondition, object (" #_obj_ ") is valid" )

# define CYG_POSTCONDITION_CLASSO( _obj_, _msg_ ) \
    CYG_POSTCONDITION( (_obj_).check_this(CYG_ASSERT_CLASS_ZEAL), _msg_)
    
# define CYG_POSTCONDITION_CLASSOC( _obj_ )       \
    CYG_POSTCONDITION_CLASSO( (_obj_),            \
       "postcondition, object (" #_obj_ ") is valid" )
                             
# define CYG_LOOP_INVARIANT_CLASSO( _obj_, _msg_) \
    CYG_LOOP_INVARIANT( (_obj_).check_this(CYG_ASSERT_CLASS_ZEAL), _msg_)

# define CYG_LOOP_INVARIANT_CLASSOC( _obj_ )      \
    CYG_LOOP_INVARIANT_CLASSO( (_obj_),           \
       "loop invariant, object (" #_obj_ ") is valid" )

// All variants of _ZERO_OR_CLASS
# define CYG_PRECONDITION_ZERO_OR_CLASS( _pobj_, _msg_ )  \
    CYG_PRECONDITION( ((0 == (_pobj_)) ||                 \
                       (_pobj_)->check_this(CYG_ASSERT_CLASS_ZEAL)), _msg_)
    
# define CYG_PRECONDITION_ZERO_OR_CLASSC( _pobj_ )        \
    CYG_PRECONDITION_ZERO_OR_CLASS( (_pobj_),             \
       "precondition, class pointer (" #_pobj_ ") is zero or valid" )
    
# define CYG_POSTCONDITION_ZERO_OR_CLASS( _pobj_, _msg_ ) \
    CYG_POSTCONDITION( ((0 == (_pobj_)) ||                \
                        (_pobj_)->check_this(CYG_ASSERT_CLASS_ZEAL)), _msg_)

# define CYG_POSTCONDITION_ZERO_OR_CLASSC( _pobj_ )       \
    CYG_POSTCONDITION_ZERO_OR_CLASS( (_pobj_),            \
       "postcondition, class pointer (" #_pobj_ ") is zero or valid" )
                             
# define CYG_LOOP_INVARIANT_ZERO_OR_CLASS( _pobj_, _msg_) \
    CYG_LOOP_INVARIANT( ((0 == (_pobj_)) ||               \
                         (_pobj_)->check_this(CYG_ASSERT_CLASS_ZEAL)), _msg_)
        
# define CYG_LOOP_INVARIANT_ZERO_OR_CLASSC( _pobj_ )      \
    CYG_LOOP_INVARIANT_ZERO_OR_CLASS( (_pobj_),           \
       "loop invariant, class pointer (" #_pobj_ ") is zero or valid" )

// All variants of _THIS
# define CYG_PRECONDITION_THIS( _msg_ )  \
    CYG_PRECONDITION( this->check_this(CYG_ASSERT_CLASS_ZEAL), _msg_)
    
# define CYG_PRECONDITION_THISC()        \
    CYG_PRECONDITION_THIS( "precondition, \"this\"  is valid" )
    
# define CYG_POSTCONDITION_THIS( _msg_ ) \
    CYG_POSTCONDITION( this->check_this(CYG_ASSERT_CLASS_ZEAL), _msg_)
    
# define CYG_POSTCONDITION_THISC()       \
    CYG_POSTCONDITION_THIS( "postcondition, \"this\" is valid" )
                             
# define CYG_LOOP_INVARIANT_THIS( _msg_) \
    CYG_LOOP_INVARIANT( this->check_this(CYG_ASSERT_CLASS_ZEAL), _msg_)
        
# define CYG_LOOP_INVARIANT_THISC()      \
    CYG_LOOP_INVARIANT_THIS( "loop invariant, \"this\" is valid" )

#endif // __cplusplus

// -------------------------------------------------------------------------
// Invariants. These are a bit more interesting. The ordinary invariants
// take an arbitrary boolean expression, and C++ does not provide any way
// of evaluating this expression automatically on entry and exit - any
// attempt to use local objects leads to trying to evaluate the expression
// when it is not in scope. This problem does not arise with objects.
//
// For C++ objects it is possible to do a bit better. A template can be
// used to create a local object whose constructor will validate the
// target object and whose destructor will validate the target object
// again. Unfortunately it is necessary to pass the type as well as
// the object: typeof() is a gcc extension, and RTTI's typeid facility
// would provide the derived class and not what we actually want.            

#ifdef CYGDBG_INFRA_DEBUG_INVARIANTS    

# define CYG_INVARIANT( _bool_, _msg_ ) \
        CYG_MACRO_START                 \
        if ( ! ( _bool_ ) )             \
            CYG_ASSERT_DOCALL( _msg_ ); \
        CYG_MACRO_END

# define CYG_INVARIANTC( _bool_ )       \
        CYG_MACRO_START                 \
        if ( ! ( _bool_ ) )             \
            CYG_ASSERT_DOCALL( "invariant (" #_bool_ ")" ); \
        CYG_MACRO_END

# ifdef __cplusplus
// NOTE: if the compiler does not manage to inline the appropriate
// template functions then the impact on code size and performance becomes
// rather large. But there are significant performance overheads anyway
// simply because of the call to check_this()...            
//
template<class X> class __CygInvariantObject {

    const X*  rep;

  private:
    // Prevent access to the default constructors.
    __CygInvariantObject() { }
    __CygInvariantObject( const __CygInvariantObject&  arg ) { }
    __CygInvariantObject & operator=( const __CygInvariantObject & arg) { return *this; }
    
  public:
    __CygInvariantObject( X* arg, const char* msg ) : rep(arg) {
        if ( !rep->check_this( CYG_ASSERT_CLASS_ZEAL ) )
            CYG_ASSERT_DOCALL( msg );
    }
    __CygInvariantObject( X& arg, const char* msg ) : rep(&arg) {
        if ( !rep->check_this( CYG_ASSERT_CLASS_ZEAL ) )
            CYG_ASSERT_DOCALL( msg );
    }
    __CygInvariantObject( const X* arg, const char* msg ) : rep(arg) {
        if ( !rep->check_this( CYG_ASSERT_CLASS_ZEAL ) )
            CYG_ASSERT_DOCALL( msg );
    }
    __CygInvariantObject( const X& arg, const char* msg ) : rep(&arg) {
        if ( !rep->check_this( CYG_ASSERT_CLASS_ZEAL ) )
            CYG_ASSERT_DOCALL( msg );
    }
    ~__CygInvariantObject( ) {
        if ( !rep->check_this( CYG_ASSERT_CLASS_ZEAL ) )
            CYG_ASSERT_DOCALL( "invariant, object valid on exit" );
        rep = 0;
    };
};

//
// These macros provide sensible concatenation facilities at
// the C preprocessor level, getting around complications in the
// macro expansion rules related to __LINE__ and __FILE__.

# define __CYG_INVARIANT_CLASSNAME_AUX( a, b) a ## b
# define __CYG_INVARIANT_CLASSNAME( a, b ) \
              __CYG_INVARIANT_CLASSNAME_AUX( a, b )


// These macro definitions do not use CYG_MACRO_START because
// I do not want the scope of the local objects to get confused.
//
// The first line of the macro expansion specifies the type of
// the local object being created. The second line invents a
// name for this object. The third line provides command-line
// arguments.    

# define CYG_INVARIANT_CLASS( _type_, _pobj_, _msg_ )          \
     __CygInvariantObject<_type_>                              \
     __CYG_INVARIANT_CLASSNAME( __invariant_class_, __LINE__ ) \
              ( _pobj_, _msg_ )

# define CYG_INVARIANT_CLASSC( _type_, _pobj_ )                \
     __CygInvariantObject<_type_>                              \
     __CYG_INVARIANT_CLASSNAME( __invariant_class_, __LINE__ ) \
              ( _pobj_, "invariant, class pointer (" #_pobj_ ") is valid" )
         
# define CYG_INVARIANT_CLASSO( _type_, _obj_, _msg_ )          \
     __CygInvariantObject<_type_>                              \
     __CYG_INVARIANT_CLASSNAME( __invariant_class_, __LINE__ ) \
              ( _obj_, _msg_ )

# define CYG_INVARIANT_CLASSOC( _type_, _obj_ )                \
     __CygInvariantObject<_type_>                              \
     __CYG_INVARIANT_CLASSNAME( __invariant_class_, __LINE__ ) \
              ( _obj_, "invariant, object (" #_obj_ ") is valid" )

# define CYG_INVARIANT_THIS( _type_, _msg_ )                   \
     __CygInvariantObject<_type_>                              \
     __CYG_INVARIANT_CLASSNAME( __invariant_class_, __LINE__ ) \
              ( this, _msg_ )
         
# define CYG_INVARIANT_THISC( _type_ )                         \
     __CygInvariantObject<_type_>                              \
     __CYG_INVARIANT_CLASSNAME( __invariant_class_, __LINE__ ) \
              ( this, "invariant, \"this\" is valid" )

# endif // __cplusplus

#else  // !CYGDBG_INFRA_DEBUG_INVARIANTS

# define CYG_INVARIANT( _bool_, _msg_ ) CYG_EMPTY_STATEMENT
# define CYG_INVARIANTC( _bool_ )       CYG_EMPTY_STATEMENT

# ifdef __cplusplus

#  define CYG_INVARIANT_CLASS( _type_, _pobj_, _msg_ )
#  define CYG_INVARIANT_CLASSC( _type_, _pobj_ )
#  define CYG_INVARIANT_CLASSO( _type_, _obj_, _msg_ )
#  define CYG_INVARIANT_CLASSOC( _type_, _obj_ )
#  define CYG_INVARIANT_THIS( _type_, _msg_ )
#  define CYG_INVARIANT_THISC( _type_ )

# endif
    
#endif // CYGDBG_INFRA_DEBUG_INVARIANTS

// -------------------------------------------------------------------------
// Compile time failure; like #error but in a macro so we can use it in
// other definitions.
//
// Usage:
// #define new CYG_COMPILETIMEFAIL( "Do NOT use new!")

#define CYG_COMPILETIMEFAIL( _msg_ ) !!!-- _msg_ --!!!


// -------------------------------------------------------------------------
// The host-side implementation of the infrastructure provides a number
// of additional functions that allow applications to provide their own
// implementation of cyg_assert_fail(). This is not strictly necessary
// since the presence of cyg_assert_fail() in the application would
// override the one in the library anyway, but it is useful to make
// certain functionality more readily available.
//
// These declarations are only available if the symbol
// CYG_DECLARE_HOST_ASSERTION_SUPPORT is defined.
#ifdef CYG_DECLARE_HOST_ASSERTION_SUPPORT

// The default assertion handler writes its output to a file and
// if possible a suitable message to stdout. It is possible to
// install an alternative handler. If this alternative returns false
// then the default handler will be invoked instead, otherwise the
// application will exit.
externC void cyg_assert_install_failure_handler(
                bool (*)(const char* /* psz_func */,
                         const char* /* psz_file */,
                         cyg_uint32  /* linenum */,
                         const char* /* psz_msg */) );

// Register a callback that should get invoked as part of handling an
// assertion failure and that will typically produce some additional
// output. For example the trace code will install a callback to output
// trace information.
//
// The first argument is a string identifying the callback. The second
// argument is a function pointer for the callback itself, whose
// argument is another function that can be invoked for output.
externC void cyg_assert_install_failure_callback(
                const char* /* name */,
                void (*)( void (*)(const char*) ));

// This function can be called by assert failure handlers to invoke
// the installed callbacks. The three arguments are function pointers
// that get invoked prior to callback invocation, by the callback itself,
// and after each callback. In the first case the argument will be the
// callback name.
externC void cyg_assert_failure_invoke_callbacks(
                void (*)(const char* /* name */),
                void (*)(const char* /* callback data */ ),
                void (*)(void) );

// This function is intended to be called from inside gdb instead of
// cyg_assert_fail(),, without the need to specify a filename or
// anything else.
externC void cyg_assert_quickfail(void);

#endif // CYG_DECLARE_HOST_ASSERTION_SUPPORT
    
// -------------------------------------------------------------------------

#endif // CYGONCE_INFRA_CYG_ASS_H multiple inclusion protection
// EOF cyg_ass.h
