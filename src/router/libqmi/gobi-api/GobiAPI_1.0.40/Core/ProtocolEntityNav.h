/*===========================================================================
FILE:
   ProtocolEntityNav.h

DESCRIPTION:
   Declaration of cProtocolEntityNav
   
PUBLIC CLASSES AND METHODS:
   cProtocolEntityNav
      This class serves as a base for all class that need to
      'navigate' a protocol entity database description.  It is
      necessary in order to seperate the structural aspects 
      from parsing/packing details

Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "CoreDatabase.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------
struct sSharedBuffer;
struct sDB2NavFragment;

// Field seperator string
extern LPCSTR PE_NAV_FIELD_SEP;

// Types of protocol entity field attributes
enum ePENavFieldAttr
{
   ePENAV_FIELD_BEGIN = -1,

   ePENAV_FIELD_NAME,         // Name of field
   ePENAV_FIELD_NAME_FULL,    // Fully qualified name of field
   ePENAV_FIELD_NAME_PARTIAL, // Partially qualified name of field
   ePENAV_FIELD_VALUE,        // Translated value of field
   ePENAV_FIELD_VALUE_RAW,    // Raw value of field
   ePENAV_FIELD_TYPE,         // Type of field
   ePENAV_FIELD_SIZE,         // Size of field
   ePENAV_FIELD_OFFSET,       // Offset of field
   ePENAV_FIELD_INDEX,        // Index of field
   ePENAV_FIELD_ID,           // ID of field

   ePENAV_FIELD_END           // Number of field attributes
};

/*===========================================================================
METHOD:
   IsValid (Inline Method)

DESCRIPTION:
   Is this a valid ePENavFieldAttr?
  
PARAMETERS:
   contentType [ I ] - The enum value being validated

RETURN VALUE:
   bool
===========================================================================*/
inline bool IsValid( ePENavFieldAttr attr )
{
   bool bRC = false;
   if (attr > ePENAV_FIELD_BEGIN && attr < ePENAV_FIELD_END)
   {
      bRC = true;
   }

   return bRC;
};

/*=========================================================================*/
// Class cProtocolEntityNav
//    Class to navigate a protocol entity
/*=========================================================================*/
class cProtocolEntityNav
{
   public:
      // Constructor
      cProtocolEntityNav( const cCoreDatabase & db );

      // Destructor
      virtual ~cProtocolEntityNav();

      // Return the fully qualified field name given the partial name
      virtual std::string GetFullFieldName( const std::string & partialName ) const;

      // Return the partial field name given the fully qualified name
      virtual std::string GetPartialFieldName( 
         const std::string &            fieldNameFQ ) const;

   protected:     
      // Evaluate the given condition
      virtual bool EvaluateCondition( 
         LPCSTR                    pCondition,
         bool &                     bResult );

      // Get the array bounds described by the fragment descriptor
      virtual bool GetArrayBounds( 
         const sDB2Fragment &       frag,
         LONGLONG &                 arraySz,
         LONGLONG &                 arrayAdj );

      // Return the value for the specified field ID as a
      // LONGLONG (field type must be able to fit)
      virtual bool GetLastValue( 
         ULONG                      fieldID,
         LONGLONG &                 val ) = 0;

      // Modify string length based on existing field value
      virtual bool ModifyStringLength( 
         const sDB2Fragment &       frag,
         sDB2Field &                field );

      // Process the protocol entity described by the given key/name
      virtual bool ProcessEntity( const std::vector <ULONG> & key );

      // (Inline) Contiue navigation now that entity has been set?
      virtual bool ContinueNavigation()
      {
         // Override to implement
         return true;
      };

      // Process a structure described by the given initial fragment
      virtual bool ProcessStruct(
         const sDB2NavFragment *       pFrag,
         const std::string &               preamble,
         LONGLONG                      arrayIndex = -1 );

      // Process the given fragment
      virtual bool ProcessFragment(
         const sDB2NavFragment *       pFrag,
         ULONG                         structOffset,
         ULONG &                       structSize,
         const std::string &               preamble );

      // Process the given field 
      virtual bool ProcessField(
         const sDB2Field *          pField,
         const std::string &            fieldName,
         LONGLONG                   arrayIndex = -1 ) = 0;
      
      // (Inline) Handle an array being entered
      virtual void EnterArray( 
         const sDB2Fragment &       /* frag */,
         LONGLONG                   /* arraySz */ )
      { };

      // (Inline) Handle an array being exited
      virtual void ExitArray( 
         const sDB2Fragment &       /* frag */,
         LONGLONG                   /* arraySz */ )
      { };

      // (Inline) Handle a structure being entered
      virtual void EnterStruct( 
         LPCSTR                    /* pName */,
         LONGLONG                   /* arrayIndex */ )
      { };

      // (Inline) Handle a structure being exited
      virtual void ExitStruct( 
         LPCSTR                    /* pName */,
         LONGLONG                   /* arrayIndex */ )
      { };

      // (Inline) Get current working offset 
      virtual ULONG GetOffset() 
      {
         // Override to implement
         return 0;
      };

      // (Inline) Set current working offset 
      virtual bool SetOffset( ULONG /* offset */ ) 
      {
         // Override to implement
         return true;
      };

      // (Inline) Get current navigation order
      virtual bool GetLSBMode() 
      {
         // Override to implement
         return true;
      };

      // (Inline) Set current navigation order
      virtual bool SetLSBMode( bool /* bLSB */ ) 
      {
         // Override to implement
         return true;
      };

      /* Generate field name strings? */
      bool mbFieldNames;

      /* Protocol entity being navigated */
      sDB2ProtocolEntity mEntity;

      /* Database reference */
      const cCoreDatabase & mDB;

      /* References to DB tables we need */
      const tDB2OptionalModMap & mConditions;
      const tDB2ExpressionModMap & mExpressions;
      const tDB2Array1ModMap & mArrays1;
      const tDB2Array2ModMap & mArrays2;

      /* Map of all 'tracked' fields */
      std::map <ULONG, std::pair <bool, LONGLONG> > mTrackedFields;
};
