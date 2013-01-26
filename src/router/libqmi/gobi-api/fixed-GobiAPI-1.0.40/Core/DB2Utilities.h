/*===========================================================================
FILE:
   DB2Utilities.h

DESCRIPTION:
   Utility functions for packing/parsing protocol entities using the
   database

PUBLIC ENUMERATIONS AND METHODS:
   sProtocolEntityKey
   sDB2PackingInput
   sDB2NavInput

   MapQMIEntityTypeToProtocolType
   MapQMIEntityTypeToQMIServiceType
   MapQMIProtocolTypeToEntityType
   DB2GetMaxBufferSize
   DB2BuildQMIBuffer
   DB2PackQMIBuffer
   DB2ReduceQMIBuffer

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
#include "SharedBuffer.h"
#include "ProtocolBuffer.h"
#include "QMIEnum.h"

#include <vector>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Struct sProtocolEntityKey
//    Simple structure to initializing protocol entity keys easier
/*=========================================================================*/
struct sProtocolEntityKey
{
   public:
      // (Inline) Constructor - default
      sProtocolEntityKey()
      { };
 
      // (Inline) Constructor - single value keys
      sProtocolEntityKey( ULONG val1 )
      {
         mKey.push_back( val1 );
      };

      // (Inline) Constructor - two value keys
      sProtocolEntityKey( 
         ULONG                      val1,
         ULONG                      val2 )
      {
         mKey.push_back( val1 );
         mKey.push_back( val2 );
      };

      // (Inline) Constructor - three value keys
      sProtocolEntityKey( 
         ULONG                      val1,
         ULONG                      val2,
         ULONG                      val3 )
      {
         mKey.push_back( val1 );
         mKey.push_back( val2 );
         mKey.push_back( val3 );
      };

      // (Inline) Constructor - psuedo-copy constructor
      sProtocolEntityKey( const std::vector <ULONG> & key )
         :  mKey( key )
      { };

      // (Inline) Constructor - copy constructor
      sProtocolEntityKey( const sProtocolEntityKey & key )
         :  mKey( key.mKey )
      { };

      // (Inline) Assignment operator
      sProtocolEntityKey & operator = ( const sProtocolEntityKey & key )
      {
         mKey = key.mKey;
         return *this;
      };

      // Cast operator to a protocol entity key
      operator std::vector <ULONG>() const
      {
         return mKey;
      };

      /* Underlying key */
      std::vector <ULONG> mKey;
};

/*=========================================================================*/
// Struct sDB2PackingInput
//    Simple structure to make dealing packing easier
/*=========================================================================*/
struct sDB2PackingInput
{
   public:
      // (Inline) Constructor - default
      sDB2PackingInput()
         :  mpData( 0 ),
            mDataLen( 0 ),
            mbString( true )
      { };
 
      // (Inline) Constructor - parameterized (string payload)
      sDB2PackingInput( 
         const sProtocolEntityKey & key,
         LPCSTR                     pValue )
         :  mKey( key ),
            mpData( 0 ),
            mDataLen( 0 ),
            mbString( true )
      { 
         if (pValue != 0 && pValue[0] != 0)
         {
            mValues = pValue;
         }
      };

      // (Inline) Constructor - parameterized (buffer payload)
      sDB2PackingInput( 
         const sProtocolEntityKey & key,
         const BYTE *               pData,
         ULONG                      dataLen )
         :  mKey( key ),
            mpData( pData ),
            mDataLen( dataLen ),
            mbString( false )
      { 
         // Nothing to do
      };

      // (Inline) Is this object in a valid state?
      bool IsValid() const
      {
         // We need a key
         if (mKey.size() <= 0)
         {
            return false;
         }

         return true;
      };

      /* Underlying key */
      std::vector <ULONG> mKey;

      /* Are the values specified by a string? */
      bool mbString;

      /* String of space delimited field values */
      std::string mValues;
      
      /* Buffer containing pre-formatted fields */
      const BYTE * mpData;

      /* Length of above buffer */
      ULONG mDataLen;
};

/*=========================================================================*/
// Struct sDB2NavInput
//    Simple structure to make dealing with key/payload easier
/*=========================================================================*/
struct sDB2NavInput
{
   public:
      // (Inline) Constructor - default
      sDB2NavInput()
         :  mpPayload( 0 ),
            mPayloadLen( 0 )
      { };

      // (Inline) Constructor - parameterized
      sDB2NavInput( 
         const std::vector <ULONG> &   key,
         const BYTE *                  pData,
         ULONG                         dataLen )
         :  mKey( key ),
            mpPayload( pData ),
            mPayloadLen( dataLen )
      { };

      // (Inline) Is this object in a valid state?
      bool IsValid() const
      {
         return (mKey.size() > 0 && mpPayload != 0 && mPayloadLen > 0);
      };

      /* Database key for payload entity */
      std::vector <ULONG> mKey;

      /* Payload */
      const BYTE * mpPayload;

      /* Size of above payload */
      ULONG mPayloadLen;
};

// Map a DB protocol entity type to a buffer protocol type
eProtocolType MapQMIEntityTypeToProtocolType( eDB2EntityType et );

// Map a DB protocol entity type to a QMI service type
eQMIService MapQMIEntityTypeToQMIServiceType( eDB2EntityType et );

// Map a buffer protocol type to a DB protocol entity type
eDB2EntityType MapQMIProtocolTypeToEntityType( 
   eProtocolType              pt,
   bool                       bIndication = false );

// Return the maximum size of a payload buffer for given type of 
// protocol entity
ULONG DB2GetMaxBufferSize( eDB2EntityType et );

// Build an allocated shared buffer for the QMI protocol
sSharedBuffer * DB2BuildQMIBuffer(
   const std::vector <sDB2NavInput> &  input );

// Build an allocated shared buffer for the QMI protocol
sSharedBuffer * DB2PackQMIBuffer(
   const cCoreDatabase &                  db,
   const std::vector <sDB2PackingInput> & input );

// Reduce a QMI buffer to DB keys and payload
std::vector <sDB2NavInput> DB2ReduceQMIBuffer( const sProtocolBuffer & buf );

