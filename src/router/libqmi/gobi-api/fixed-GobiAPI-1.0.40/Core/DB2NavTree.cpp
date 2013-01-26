/*===========================================================================
FILE:
   ProtocolEntityNavTree.cpp

DESCRIPTION:
   Implementation of cDB2NavTree
   
PUBLIC CLASSES AND METHODS:
   sDB2NavFragment
   cDB2NavTree
      This class distills the database description of a protocol
      entity into a simple tree structure more suited to
      efficient navigation for parsing/packing

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
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "DB2NavTree.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// sDB2NavFragment Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sDB2NavFragment (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
sDB2NavFragment::sDB2NavFragment()
   :  mpField( 0 ),
      mpFragment( 0 ),
      mpNextFragment( 0 ),
      mpLinkFragment( 0 )
{
   // Nothing to do
}

/*=========================================================================*/
// cDB2NavTree Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cDB2NavTree (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   db             [ I ] - Database to use
  
RETURN VALUE:
   None
===========================================================================*/
cDB2NavTree::cDB2NavTree( const cCoreDatabase & db )
   :  mDB( db )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   ~cDB2NavTree (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cDB2NavTree::~cDB2NavTree()
{
   // Clean-up our fragment allocations
   std::list <sDB2NavFragment *>::iterator pIter;
   for (pIter = mFragments.begin(); pIter != mFragments.end(); pIter++)
   {
      sDB2NavFragment * pFrag = *pIter;
      if (pFrag != 0)
      {
         delete pFrag;
      }
   }
}

/*===========================================================================
METHOD:
   BuildTree (Internal Method)

DESCRIPTION:
   Build nav tree for the entity described by the given key/name

PARAMETERS:
   key         [ I ] - Key into the protocol entity table
  
RETURN VALUE:
   bool
===========================================================================*/
bool cDB2NavTree::BuildTree( const std::vector <ULONG> & key )
{
   // Assume failure
   bool bRC = false;

   // Look up entity definition
   bool bFound = mDB.FindEntity( key, mEntity );

   // Did we find it?
   if (bFound == false)
   {
      // No definition in database
      return bRC;
   }

   // A structure to navigate?
   if (mEntity.mStructID == -1)
   {
      bRC = true;
      return bRC;
   }

   const tDB2FragmentMap & structTable = mDB.GetProtocolStructs();

   // Grab first fragment of structure
   std::pair <ULONG, ULONG> id( mEntity.mStructID, 0 );
   tDB2FragmentMap::const_iterator pFrag = structTable.find( id );

   // Nothing to navigate?
   if (pFrag == structTable.end())
   {
      ASSERT( 0 );
      return bRC;
   }

   // No name?
   if (mEntity.mpName == 0 || mEntity.mpName[0] == 0)
   {
      ASSERT( 0 );
      return bRC;
   }

   // Process the initial structure
   bRC = ProcessStruct( &pFrag->second, 0 );
   return bRC;
}

/*===========================================================================
METHOD:
   ProcessStruct (Internal Method)

DESCRIPTION:
   Process a structure described by the given initial fragment

PARAMETERS:
   frag        [ I ] - Entry point for structure
   pOwner      [ I ] - Owning fragment
  
RETURN VALUE:
   bool
===========================================================================*/
bool cDB2NavTree::ProcessStruct( 
   const sDB2Fragment *       pFrag,
   sDB2NavFragment *          pOwner )
{
   // Assume success
   bool bRC = true;

   // Grab struct ID/fragment ID from fragment
   std::pair <ULONG, ULONG> key = pFrag->GetKey();
   ULONG structID = key.first;

   const tDB2FragmentMap & structTable = mDB.GetProtocolStructs();
   const tDB2FieldMap & fieldTable = mDB.GetProtocolFields();

   // Sync iterator to fragment
   tDB2FragmentMap::const_iterator pFragIter = structTable.find( key );
   if (pFragIter == structTable.end())
   {
      // This should not happen
      ASSERT( 0 );

      bRC = false;
      return bRC;
   }

   // Fragments we allocate along the way
   sDB2NavFragment * pOld = 0;
   sDB2NavFragment * pNew = 0;

   // Process each fragment in the structure
   while ( (pFragIter != structTable.end())
      &&   (pFragIter->second.mStructID == structID) )
   {      
      pFrag = &pFragIter->second;

      // Allocate our new fragment
      pNew = new sDB2NavFragment;
      if (pNew == 0)
      {
         bRC = false;
         break;
      }
      
      // Store DB fragemnt
      pNew->mpFragment = pFrag;
      mFragments.push_back( pNew );

      // Hook previous up to us
      if (pOld != 0 && pOld->mpNextFragment == 0)
      {
         pOld->mpNextFragment = pNew;
      }

      // Hook owner up to us
      if (pOwner != 0 && pOwner->mpLinkFragment == 0)
      {
         pOwner->mpLinkFragment = pNew;
      }   

      // Modified?
      switch (pFrag->mModifierType)
      {
         case eDB2_MOD_VARIABLE_ARRAY:
         case eDB2_MOD_VARIABLE_STRING1:
         case eDB2_MOD_VARIABLE_STRING2:
         case eDB2_MOD_VARIABLE_STRING3:
         {
            const tDB2Array1ModMap & arrays1 = mDB.GetArray1Mods();

            tDB2Array1ModMap::const_iterator pTmp;
            pTmp = arrays1.find( pFrag->mpModifierValue );

            if (pTmp != arrays1.end())
            {
               // We need to track the value of the given field
               std::pair <bool, LONGLONG> entry( false, 0 );
               mTrackedFields[pTmp->second] = entry;
            }
            else
            {
               bRC = false;
            }
         }
         break;

         case eDB2_MOD_VARIABLE_ARRAY2:
         {
            const tDB2Array2ModMap & arrays2 = mDB.GetArray2Mods();

            tDB2Array2ModMap::const_iterator pTmp;
            pTmp = arrays2.find( pFrag->mpModifierValue );

            if (pTmp != arrays2.end())
            {
               // We need to track the value of the given fields
               std::pair <bool, LONGLONG> entry( false, 0 );
               mTrackedFields[pTmp->second.first] = entry;
               mTrackedFields[pTmp->second.second] = entry;
            }
            else
            {
               bRC = false;
            }
         }
         break;

         case eDB2_MOD_OPTIONAL:
         {
            const tDB2OptionalModMap & conditions = mDB.GetOptionalMods();

            tDB2OptionalModMap::const_iterator pTmp;
            pTmp = conditions.find( pFrag->mpModifierValue );

            if (pTmp != conditions.end())
            {
               const sDB2SimpleCondition & con = pTmp->second;

               // We need to track the value of the given field
               std::pair <bool, LONGLONG> entry( false, 0 );
               mTrackedFields[con.mID] = entry;

               if (con.mbF2F == true)
               {
                  // We need to track the value of the given field
                  std::pair <bool, LONGLONG> entry( false, 0 );
                  mTrackedFields[(ULONG)con.mValue] = entry;
               }
            }
            else
            {
               bRC = false;
            }
         }
         break;

         case eDB2_MOD_VARIABLE_ARRAY3:
         {
            const tDB2ExpressionModMap & exprs = mDB.GetExpressionMods();

            tDB2ExpressionModMap::const_iterator pTmp;
            pTmp = exprs.find( pFrag->mpModifierValue );

            if (pTmp != exprs.end())
            {
               const sDB2SimpleExpression & expr = pTmp->second;

               // We need to track the value of the given field
               std::pair <bool, LONGLONG> entry( false, 0 );
               mTrackedFields[expr.mID] = entry;

               if (expr.mbF2F == true)
               {
                  // We need to track the value of the given field
                  std::pair <bool, LONGLONG> entry( false, 0 );
                  mTrackedFields[(ULONG)expr.mValue] = entry;
               }
            }
            else
            {
               bRC = false;
            }
         }
         break;
      };

      // What type of fragment is this?
      switch (pFrag->mFragmentType)
      {
         case eDB2_FRAGMENT_FIELD:
         {
            // Grab field ID
            ULONG fieldID = pFrag->mFragmentValue;

            // Find field representation in database
            tDB2FieldMap::const_iterator pField = fieldTable.find( fieldID );
            if (pField != fieldTable.end())
            {
               pNew->mpField = &pField->second;
            }
            else
            {
               bRC = false;
            }
         }
         break;

         case eDB2_FRAGMENT_STRUCT:                     
         {
            // Grab structure ID
            ULONG structID = pFrag->mFragmentValue;

            // Grab first fragment of structure
            std::pair <ULONG, ULONG> id( structID, 0 );
            tDB2FragmentMap::const_iterator pFragIterTmp;
            pFragIterTmp = structTable.find( id );
            if (pFragIterTmp != structTable.end())
            {        
               pFrag = &pFragIterTmp->second;    
               bRC = ProcessStruct( pFrag, pNew );
            }
            else
            {
               bRC = false;
            }
         }
         break;

         case eDB2_FRAGMENT_VARIABLE_PAD_BITS:
         case eDB2_FRAGMENT_VARIABLE_PAD_BYTES:
         {
            // We need to track the value of the given field
            std::pair <bool, LONGLONG> entry( false, 0 );
            mTrackedFields[pFrag->mFragmentValue] = entry;

            bRC = true;
         }
         break;
         
         default:
            bRC = true;
            break;
      }

      if (bRC == true)
      {
         pFragIter++;

         pOld = pNew;
         pNew = 0;
      }
      else
      {
         break;
      }
   }

   return bRC;
}
