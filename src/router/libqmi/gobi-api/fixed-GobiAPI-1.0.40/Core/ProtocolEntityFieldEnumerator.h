/*===========================================================================
FILE:
   ProtocolEntityFieldEnumerator.h

DESCRIPTION:
   Declaration of cProtocolEntityFieldEnumerator
   
PUBLIC CLASSES AND METHODS:
   cProtocolEntityFieldEnumerator
      Class to navigate a protoocl entity, generating a vector of
      field IDs, i.e. every field referenced by this protocol entity
      in the exact order it would regularly be found

      NOTE: This only functions for fixed structures such as NV items

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
#include "ProtocolEntityNav.h"
#include "DB2NavTree.h"

#include <vector>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Class cProtocolEntityFieldEnumerator
/*=========================================================================*/
class cProtocolEntityFieldEnumerator : public cProtocolEntityNav
{
   public:
      // (Inline) Constructor
      cProtocolEntityFieldEnumerator( 
         const cCoreDatabase &         db,
         const std::vector <ULONG> &   key )
         :  cProtocolEntityNav( db ),
            mKey( key )
      {
         // Nothing to do
      };

      // (Inline) Destructor
      virtual ~cProtocolEntityFieldEnumerator() 
      { 
         // Nothing to do
      };

      // (Inline) Enumerate the fields
      virtual bool Enumerate()
      {
         bool bRC = ProcessEntity( mKey );
         return bRC;
      };

      // (Inline) Return fields
      const std::vector <ULONG> & GetFields() const
      {
         return mFields;
      };

   protected:     
      // (Inline) Evaluate the given condition
      virtual bool EvaluateCondition( 
         LPCSTR                    /* pCondition */,
         bool &                     bResult )
      {
         // All conditions pass
         bResult = true;
         return bResult;
      };

      // Return the value for the specified field ID as a
      // LONGLONG (field type must be able to fit)
      virtual bool GetLastValue( 
         ULONG                      /* fieldID */,
         LONGLONG &                 val )
      {
         // This should only be called for figuring out array
         // boundaries (including strings)
         val = 1;
         return true;
      };

      // Process the given field 
      virtual bool ProcessField(
         const sDB2Field *          pField,
         const std::string &            /* fieldName */,
         LONGLONG                   /* arrayIndex = -1 */ )
      {
         if (pField != 0)
         {
            mFields.push_back( pField->mID );
         }

         return true;
      };


      /* Protocol entity being navigated */
      std::vector <ULONG> mKey;

      /* Fields (by ID) */
      std::vector <ULONG> mFields;
};
