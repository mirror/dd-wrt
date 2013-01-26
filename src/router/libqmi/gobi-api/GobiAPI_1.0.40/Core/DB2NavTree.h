/*===========================================================================
FILE:
   ProtocolEntityNavTree.h

DESCRIPTION:
   Declaration of cProtocolEntityNavTree
   
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
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "CoreDatabase.h"

#include <list>
#include <map>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Struct sDB2NavFragment
//    Protocol entity navigation fragment
/*=========================================================================*/
struct sDB2NavFragment
{
   public:
      // Constructor
      sDB2NavFragment();

      /* Associated DB fragment (never empty) */
      const sDB2Fragment * mpFragment;

      /* Associated DB field (may be empty) */
      const sDB2Field * mpField;

      /* Next fragment in this structure */
      const sDB2NavFragment * mpNextFragment;

      /* Fragment linked to this structure */
      const sDB2NavFragment * mpLinkFragment;
};

/*=========================================================================*/
// Class cDB2NavTree
//    Class to describe a protocol entity suited to efficient navigation
/*=========================================================================*/
class cDB2NavTree
{
   public:
      // Constructor
      cDB2NavTree( const cCoreDatabase & db );

      // Destructor
      virtual ~cDB2NavTree();

      // Build nav tree for the protocol entity described by the given key
      bool BuildTree( const std::vector <ULONG> & key );

      // (Inline) Return protocol entity
      const sDB2ProtocolEntity & GetEntity() const
      {
         return mEntity;
      };

      // (Inline) Return fragments
      const std::list <sDB2NavFragment *> & GetFragments() const
      {
         return mFragments;
      };

      // Return a map of all tracked fields
      std::map <ULONG, std::pair <bool, LONGLONG> > GetTrackedFields() const
      {
         return mTrackedFields;
      };

   protected:     
      // Process a structure described by the given initial fragment
      bool ProcessStruct( 
         const sDB2Fragment *       pFrag,
         sDB2NavFragment *          pOwner );
      
      /* Protocol entity being navigated */
      sDB2ProtocolEntity mEntity;

      /* Database reference */
      const cCoreDatabase & mDB;

      /* List of all allocated fragments */
      std::list <sDB2NavFragment *> mFragments;

      /* Map of all 'tracked' fields */
      std::map <ULONG, std::pair <bool, LONGLONG> > mTrackedFields;      
};


