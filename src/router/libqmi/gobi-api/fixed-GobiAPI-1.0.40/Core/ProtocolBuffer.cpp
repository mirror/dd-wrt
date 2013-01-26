/*===========================================================================
FILE:
   ProtocolBuffer.cpp

DESCRIPTION:
   Generic protocol structures and affliated methods
   
PUBLIC CLASSES AND METHODS:
   sProtocolBuffer
      Simple struct to represent a protocol buffer using a reference counted
      (shared) buffer, this allows us to use in in several places without
      copying it once in each place.  A few base services are provided
      but the main purpose is to provide a class to inherit off of for
      specific protocols

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
#include "ProtocolBuffer.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// sProtocolBuffer Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   sProtocolBuffer (Public Method)

DESCRIPTION:
   Constructor (default)
  
RETURN VALUE:
   None
===========================================================================*/
sProtocolBuffer::sProtocolBuffer()
   :  mpData( 0 ),
      mbValid( false )
{
   // Object is currently invalid
   mTimestamp = EMPTY_TIME;
}

/*===========================================================================
METHOD:
   sProtocolBuffer (Public Method)

DESCRIPTION:
   Constructor (parameterized)

PARAMETERS:
   pBuffer     [ I ] - Shareable buffer that contains the DIAG data
  
RETURN VALUE:
   None
===========================================================================*/
sProtocolBuffer::sProtocolBuffer( sSharedBuffer * pBuffer )
   :  mpData( 0 ),
      mbValid( false )
{
   mTimestamp = EMPTY_TIME;
 
   time_t rawtime;
   time( &rawtime );
   tm * timestamp = localtime( &rawtime );
   if (timestamp != 0)
   {
      mTimestamp = *timestamp;
   }

   if (mpData != 0 && mpData->IsValid() == true)
   {
      mpData->Release();
      mpData = 0;
   }  

   mpData = pBuffer;
   if (mpData != 0 && mpData->IsValid() == true)
   {
      mpData->AddRef();
   }
   else
   {
      mpData = 0;
   }
    
   // NOTE: Derived classes need to call their own validation method
   // in their constructors since the override might try to access
   // data that is not yet in place
   sProtocolBuffer::Validate();
}

/*===========================================================================
METHOD:
   sProtocolBuffer (Public Method)

DESCRIPTION:
   Copy constructor

PARAMETERS:
   copyThis    [ I ] - sProtocolBuffer to base the new one on
  
RETURN VALUE:
   None
===========================================================================*/
sProtocolBuffer::sProtocolBuffer( const sProtocolBuffer & copyThis )
   :  mpData( copyThis.mpData ),
      mTimestamp( copyThis.mTimestamp ),
      mbValid( copyThis.mbValid )
{
   // Bump reference count for shared buffer
   if (mpData != 0 && mpData->IsValid() == true)
   {
      mpData->AddRef();
   }
   else
   {
      mpData = 0;
      mbValid = false;
   }   
}

/*===========================================================================
METHOD:
   operator = (Public Method)

DESCRIPTION:
   Assignment operator

PARAMETERS:
   copyThis    [ I ] - sProtocolBuffer to base the new one on

RETURN VALUE:
   sProtocolBuffer &
===========================================================================*/
sProtocolBuffer & sProtocolBuffer::operator = ( const sProtocolBuffer & copyThis )
{
   // Do we already have data?
   if (mpData != 0)
   {
      // Is it different than what we are duplicating?
      if (mpData != copyThis.mpData)
      {
         // Yes, release our current buffer
         mpData->Release();
      }
   }

   mpData     = copyThis.mpData;
   mTimestamp = copyThis.mTimestamp;
   mbValid    = copyThis.mbValid;

   // Bump reference count for shared buffer
   if (mpData != 0 && mpData->IsValid() == true)
   {
      mpData->AddRef();
   }
   else
   {
      mpData = 0;
      mbValid = false;
   }

   return *this;
}

/*===========================================================================
METHOD:
   ~sProtocolBuffer (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
sProtocolBuffer::~sProtocolBuffer() 
{
   if (mpData != 0 && mpData->IsValid() == true)
   {
      mpData->Release();
      mpData = 0;
   }  
   else if (mpData != 0)
   {
      ASSERT( 0 );
   }

   mbValid = false;
}
