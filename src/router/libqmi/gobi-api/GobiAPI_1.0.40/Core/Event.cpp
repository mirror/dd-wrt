/*===========================================================================
FILE:
   Event.cpp

DESCRIPTION:
   Implementation of cEvent class
   
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
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "Event.h"

/*===========================================================================
METHOD:
   WaitOnMultipleEvents (Free Method)
   
DESCRIPTION:
   Wait for any of the events to be set and return the value

   Note: If multiple events are set, only the event specified by
      eventIndex will be read from.  Run this function again
      to get the next event.

PARAMETERS:
   events      [ I ] - Vector of events which may be signaled
   timeoutMS   [ I ] - Relative timeout length (in milliseconds)
   val         [ O ] - Associated value upon success
   eventIndex  [ O ] - Index of event which was signaled
   
RETURN VALUE:
   Return code
      positive for number of events set
      -ETIME on timeout
      negative errno value on failure
===========================================================================*/
int WaitOnMultipleEvents(
   std::vector <cEvent *>        events,
   DWORD                         timeoutMS, 
   DWORD &                       val,
   DWORD &                       eventIndex )
{
   // Check internal pipes' status
   for (int index = 0; index < events.size(); index++)
   {
      int error = events[index]->mError;
      if (error != 0)
      {
         TRACE( "cEvent %d has error %d\n", index, error );
         return -error;
      }
   }

   // Initialize the FD set
   fd_set fds;
   FD_ZERO( &fds );

   // Add each item to the FD set, keeping track of the largest,
   // which is used for select()
   int largestFD = 0;
   for (int index = 0; index < events.size(); index++)
   {
      int pipe = events[index]->mPipes[READING];
      FD_SET( pipe, &fds );

      largestFD = std::max( pipe, largestFD );
   }
   
   struct timeval timeOut;

   // Add avoiding an overflow on (long)usec
   timeOut.tv_sec = timeoutMS / 1000l;
   timeOut.tv_usec = ( timeoutMS % 1000l ) * 1000l;

   // Wait for activity on the pipes for the specified amount of time
   int rc = select( largestFD + 1, &fds, 0, 0, &timeOut );
   if (rc == -1)
   {
      TRACE( "WaitOnMultipleEvents error %d\n", errno );
      return -errno;
   }
   else if (rc == 0)
   {
      // No activity on the pipes
      return -ETIME;
   }

   int numSignaled = rc;

   // Only read from first pipe which was signaled
   int signaled = -1;
   for (int index = 0; index < events.size(); index++)
   {
      int pipe = events[index]->mPipes[READING];
      if (FD_ISSET( pipe, &fds ) != 0)
      {
         signaled = index;
         break;
      }
   }

   if (signaled == -1)
   {
      // Odd, no one was signaled
      return -ENODATA;
   }

   DWORD tempVal = 0;
   rc = events[signaled]->Read( tempVal );
   if (rc == 0)
   {
      // Success
      val = tempVal;
      eventIndex = signaled;
      return numSignaled;
   }
   else
   {
      // failure
      return rc;
   }
}

/*=========================================================================*/
// cEvent Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cEvent (Public Method)
   
DESCRIPTION:
   Constructor

RETURN VALUE:
   None
===========================================================================*/
cEvent::cEvent()
   :   mError( 0 )
{
   int rc = pipe( mPipes );
   if (rc != 0)
   {
      mError = errno;
      TRACE( "cEvent - Error %d creating pipe, %s\n", 
             mError, 
             strerror( mError ) );
   }
}

/*===========================================================================
METHOD:
   ~cEvent (Public Method)
   
DESCRIPTION:
   Destructor

RETURN VALUE:
   None
===========================================================================*/
cEvent::~cEvent()
{
   // Check internal pipe status
   if (mError == 0)
   {
      Close();
      mError = EBADF;
   }
}

/*===========================================================================
METHOD:
   Close (Internal Method)
   
DESCRIPTION:
   Close pipe

RETURN VALUE:
   Return code
      0 on success
      errno value on failure
===========================================================================*/
int cEvent::Close()
{
   int retCode = 0;

   int rc = close( mPipes[READING] );
   mPipes[READING] = -1;

   if (rc != 0)
   {
      retCode = errno;
      TRACE( "cEvent - Error %d deleting pipe[READING], %s\n", 
             retCode, 
             strerror( retCode ) );
   }

   rc = close( mPipes[WRITING] );
   mPipes[WRITING] = -1;

   if (rc != 0)
   {
      retCode = errno;
      TRACE( "cEvent - Error %d deleting pipe[WRITING], %s\n", 
             retCode, 
             strerror( retCode ) );
   }
   
   return retCode;
}

/*===========================================================================
METHOD:
   Set (Public Method)
   
DESCRIPTION:
   Set/signal the event with the specified value

PARAMETERS:
   val         [ I ] - Value to pass through with signal
   
RETURN VALUE:
   Return code
      0 on success
      errno value on failure
===========================================================================*/
int cEvent::Set( DWORD val )
{
   // Check internal pipe status
   if (mError != 0)
   {
      return mError;
   }

   PBYTE pWrite = (PBYTE)&val;

   int writeSize = sizeof( DWORD );
   while (writeSize > 0)
   {
      int bytesWritten = write( mPipes[WRITING], pWrite, writeSize );
      if (bytesWritten == -1)
      {
         // Store error from write
         int writeErr = errno;

         // First error?
         if (mError == 0)
         {
            // Yes, save the error
            mError = writeErr;
         }

         // We cannot recover from this error
         Close();
         return writeErr;
      }
      
      pWrite += bytesWritten;
      writeSize -= bytesWritten;
   }      

   // Success
   return 0;
}
      
/*===========================================================================
METHOD:
   Wait (Free Method)
   
DESCRIPTION:
   Wait for the event to be signalled and return the read in value

PARAMETERS:
   timeoutMS   [ I ] - Relative timeout length (in milliseconds)
   val         [ O ] - Associated value upon success
   
RETURN VALUE:
   Return code
      0 on success
      ETIME on timeout
      errno value on failure
===========================================================================*/
int cEvent::Wait( 
   DWORD                      timeoutMS, 
   DWORD &                    val )
{
   // Check internal pipe status
   if (mError != 0)
   {
      return mError;
   }

   fd_set fds;
   FD_ZERO( &fds );
   FD_SET( mPipes[READING], &fds );
   
   struct timeval timeOut;

   // Add avoiding an overflow on (long)usec
   timeOut.tv_sec = timeoutMS / 1000l;
   timeOut.tv_usec = ( timeoutMS % 1000l ) * 1000l;

   // Wait for activity on the pipe for the specified amount of time
   int rc = select( mPipes[READING] + 1, &fds, 0, 0, &timeOut );
   if (rc == -1)
   {
      // Store error from select
      int selectErr = errno;

      // First error?
      if (mError == 0)
      {
         // Yes, save the error
         mError = selectErr;
      }

      // We cannot recover from this error
      Close();
      return selectErr;
   }
   else if (rc == 0)
   {
      // No activity on the pipe
      return ETIME;
   }

   return Read( val );
}

/*===========================================================================
METHOD:
   Clear (Free Method)
   
DESCRIPTION:
   Read and discard all values currently in the pipe
===========================================================================*/
void cEvent::Clear()
{
   DWORD unusedVal;
   int rc = 0;
   while (rc == 0)
   {
      rc = Wait( (DWORD)0, unusedVal );
   }
}


/*===========================================================================
METHOD:
   Read (Internal Method)
   
DESCRIPTION:
   Read a DWORD from the pipe

RETURN VALUE:
   Return code
      0 on success
      errno value on failure
===========================================================================*/
int cEvent::Read( DWORD & val )
{
   DWORD tempVal;
   PBYTE pRead = (PBYTE)&tempVal;

   int readSize = sizeof( DWORD );
   while (readSize > 0)
   {
      int bytesRead = read( mPipes[READING], pRead, readSize );
      if (bytesRead <= 0)
      {
         // Store error from read
         int readErr = errno;
         if (readErr == 0)
         {
            // Hard error! This should NEVER happen for a pipe
            ASSERT( 0 );
            readErr = EBADF;
         }

         // First error?
         if (mError == 0)
         {
            // Yes, store the error
            mError = readErr;
         }

         // We cannot recover from this error
         Close();
         return readErr;
      }
      
      pRead += bytesRead;
      readSize -= bytesRead;
   }

   val = tempVal;
   
   return 0;
}