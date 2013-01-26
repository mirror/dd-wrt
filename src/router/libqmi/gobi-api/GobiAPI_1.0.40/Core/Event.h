/*===========================================================================
FILE:
   Event.h

DESCRIPTION:
   Declaration of cEvent class
   
PUBLIC CLASSES AND METHODS:
   WaitOnMultipleEvents
   cEvent
      Functionality to mimic Windows events using UNIX pipes (enhanced
      somewhat to allow one to specify a DWORD value to pass through
      when signalling the event)

   WARNING:
      This class is not designed to be thread safe

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
#include "StdAfx.h"
#include <vector>

//---------------------------------------------------------------------------
// Prototype
//---------------------------------------------------------------------------

class cEvent;

/*=========================================================================*/
// Free methods
/*=========================================================================*/

// Wait for any of the events to be set and return the value
int WaitOnMultipleEvents(
   std::vector <cEvent *>        events,
   DWORD                         timeoutMS, 
   DWORD &                       val,
   DWORD &                       eventIndex );

/*=========================================================================*/
// Class cEvent
/*=========================================================================*/
class cEvent
{
   public:
      // Constructor
      cEvent();
      
      // Destructor
      ~cEvent();

      // Set/signal the event with the specified value
      int Set( DWORD val );

      // Wait for the event to be signalled and return the read in value
      int Wait( 
         DWORD                      timeoutMS, 
         DWORD &                    val );

      // Read and discard all values currently in the pipe
      void Clear();

   protected:
      // Close pipe (used in errors or normal exit)
      int Close();

      // Read from the pipe
      int Read( DWORD & val );

      /* Internal error status */
      int mError;
      
      /*  Internal pipes */
      int mPipes[2];

      // WaitOnMultipleEvents gets full access
      friend int WaitOnMultipleEvents(
         std::vector <cEvent *>        events,
         DWORD                         timeoutMS, 
         DWORD &                       val,
         DWORD &                       eventIndex );
};

